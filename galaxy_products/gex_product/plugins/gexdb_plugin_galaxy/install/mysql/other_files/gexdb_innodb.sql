-- MySQL Administrator dump 1.4
--
-- ------------------------------------------------------
-- Server version	5.0.19-nt-log


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;

/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;


--
-- Create schema gexdb
--

CREATE DATABASE /*!32312 IF NOT EXISTS*/ gexdb;
USE gexdb;

DROP TABLE IF EXISTS `et_hbin`;
CREATE TABLE `et_hbin` (
  `hbin_id` int(10) unsigned NOT NULL auto_increment,
  `splitlot_id` int(10) unsigned NOT NULL default '0',
  `hbin_no` smallint(5) unsigned NOT NULL default '0',
  `hbin_name` varchar(255) NOT NULL default '',
  `hbin_cat` char(1) default NULL,
  `nb_parts` int(10) unsigned NOT NULL default '0',
  `nb_runs` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`hbin_id`),
  UNIQUE KEY `uk_et_hbin_1` (`splitlot_id`,`hbin_no`),
  CONSTRAINT `fk_et_hbin_1` FOREIGN KEY (`splitlot_id`) REFERENCES `et_splitlot` (`splitlot_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40000 ALTER TABLE `et_hbin` DISABLE KEYS */;
/*!40000 ALTER TABLE `et_hbin` ENABLE KEYS */;


DROP TABLE IF EXISTS `et_lot`;
CREATE TABLE `et_lot` (
  `lot_id` varchar(255) NOT NULL default '',
  `product_name` varchar(255) default NULL,
  `nb_parts` int(10) unsigned NOT NULL default '0',
  `nb_parts_good` int(10) unsigned NOT NULL default '0',
  `nb_runs` int(10) unsigned NOT NULL default '0',
  `nb_runs_good` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`lot_id`),
  KEY `fk_et_lot_1` (`product_name`),
  CONSTRAINT `fk_et_lot_1` FOREIGN KEY (`product_name`) REFERENCES `product` (`product_name`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40000 ALTER TABLE `et_lot` DISABLE KEYS */;
/*!40000 ALTER TABLE `et_lot` ENABLE KEYS */;


DROP TABLE IF EXISTS `et_ptest_info`;
CREATE TABLE `et_ptest_info` (
  `ptest_info_id` int(10) unsigned NOT NULL auto_increment,
  `splitlot_id` int(10) unsigned NOT NULL default '0',
  `tnum` int(10) unsigned NOT NULL default '0',
  `tname` varchar(255) NOT NULL default '',
  `units` varchar(255) NOT NULL default '',
  `flags` binary(1) NOT NULL default '\0',
  `ll` float default NULL,
  `hl` float default NULL,
  `testseq` smallint(5) unsigned default NULL,
  PRIMARY KEY  (`ptest_info_id`),
  UNIQUE KEY `uk_et_ptest_info_1` (`splitlot_id`,`tnum`,`tname`),
  CONSTRAINT `fk_et_ptest_info_1` FOREIGN KEY (`splitlot_id`) REFERENCES `et_splitlot` (`splitlot_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40000 ALTER TABLE `et_ptest_info` DISABLE KEYS */;
/*!40000 ALTER TABLE `et_ptest_info` ENABLE KEYS */;


DROP TABLE IF EXISTS `et_ptest_limits`;
CREATE TABLE `et_ptest_limits` (
  `ptest_info_id` int(10) unsigned NOT NULL default '0',
  `site_no` smallint(5) unsigned NOT NULL default '255',
  `splitlot_id` int(10) unsigned NOT NULL default '0',
  `ll` float NOT NULL default '0',
  `hl` float NOT NULL default '0',
  UNIQUE KEY `uk_et_ptest_limits_1` (`ptest_info_id`,`site_no`,`splitlot_id`),
  KEY `fk_et_ptest_limits_2` (`splitlot_id`),
  CONSTRAINT `fk_et_ptest_limits_1` FOREIGN KEY (`ptest_info_id`) REFERENCES `et_ptest_info` (`ptest_info_id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_et_ptest_limits_2` FOREIGN KEY (`splitlot_id`) REFERENCES `et_splitlot` (`splitlot_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40000 ALTER TABLE `et_ptest_limits` DISABLE KEYS */;
/*!40000 ALTER TABLE `et_ptest_limits` ENABLE KEYS */;


DROP TABLE IF EXISTS `et_result_ptest`;
CREATE TABLE `et_result_ptest` (
  `ptest_info_id` int(10) unsigned NOT NULL default '0',
  `run_id` int(10) unsigned NOT NULL default '255',
  `flags` binary(1) NOT NULL default '\0',
  `VALUE` float default NULL,
  KEY `fk_et_result_ptest_1` (`ptest_info_id`),
  KEY `fk_et_result_ptest_2` (`run_id`),
  CONSTRAINT `fk_et_result_ptest_1` FOREIGN KEY (`ptest_info_id`) REFERENCES `et_ptest_info` (`ptest_info_id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_et_result_ptest_2` FOREIGN KEY (`run_id`) REFERENCES `et_run` (`run_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40000 ALTER TABLE `et_result_ptest` DISABLE KEYS */;
/*!40000 ALTER TABLE `et_result_ptest` ENABLE KEYS */;


DROP TABLE IF EXISTS `et_run`;
CREATE TABLE `et_run` (
  `run_id` int(10) unsigned NOT NULL auto_increment,
  `splitlot_id` int(10) unsigned NOT NULL default '0',
  `site_no` smallint(5) unsigned NOT NULL default '255',
  `part_id` varchar(255) default NULL,
  `part_x` smallint(6) default NULL,
  `part_y` smallint(6) default NULL,
  `hbin_no` smallint(5) unsigned NOT NULL default '0',
  `sbin_no` smallint(5) unsigned default NULL,
  `ttime` int(10) unsigned default NULL,
  `tests_executed` smallint(5) unsigned NOT NULL default '0',
  `tests_failed` smallint(5) unsigned NOT NULL default '0',
  `firstfail_tnum` int(10) unsigned default NULL,
  `firstfail_tname` varchar(255) default NULL,
  `retest_index` tinyint(3) unsigned NOT NULL default '0',
  PRIMARY KEY  (`run_id`),
  KEY `fk_et_run_1` (`splitlot_id`),
  CONSTRAINT `fk_et_run_1` FOREIGN KEY (`splitlot_id`) REFERENCES `et_splitlot` (`splitlot_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40000 ALTER TABLE `et_run` DISABLE KEYS */;
/*!40000 ALTER TABLE `et_run` ENABLE KEYS */;


DROP TABLE IF EXISTS `et_sbin`;
CREATE TABLE `et_sbin` (
  `sbin_id` int(10) unsigned NOT NULL auto_increment,
  `splitlot_id` int(10) unsigned NOT NULL default '0',
  `sbin_no` smallint(5) unsigned NOT NULL default '0',
  `sbin_name` varchar(255) NOT NULL default '',
  `sbin_cat` char(1) default NULL,
  `nb_parts` int(10) unsigned NOT NULL default '0',
  `nb_runs` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`sbin_id`),
  UNIQUE KEY `uk_et_sbin_1` (`splitlot_id`,`sbin_no`),
  CONSTRAINT `fk_et_sbin_1` FOREIGN KEY (`splitlot_id`) REFERENCES `et_splitlot` (`splitlot_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40000 ALTER TABLE `et_sbin` DISABLE KEYS */;
/*!40000 ALTER TABLE `et_sbin` ENABLE KEYS */;


DROP TABLE IF EXISTS `et_splitlot`;
CREATE TABLE `et_splitlot` (
  `splitlot_id` int(10) unsigned NOT NULL auto_increment,
  `lot_id` varchar(255) NOT NULL default '0',
  `sublot_id` varchar(255) NOT NULL default '',
  `wafer_id` varchar(255) default NULL,
  `setup_t` int(10) unsigned NOT NULL default '0',
  `start_t` int(10) unsigned NOT NULL default '0',
  `finish_t` int(10) unsigned NOT NULL default '0',
  `stat_num` tinyint(3) unsigned NOT NULL default '0',
  `tester_name` varchar(255) NOT NULL default '',
  `flags` binary(2) NOT NULL default '\0\0',
  `nb_parts` int(10) unsigned NOT NULL default '0',
  `nb_parts_good` int(10) unsigned NOT NULL default '0',
  `nb_runs` int(10) unsigned NOT NULL default '0',
  `nb_runs_good` int(10) unsigned NOT NULL default '0',
  `data_origin` tinyint(3) unsigned NOT NULL default '0',
  `retest_index` tinyint(3) unsigned NOT NULL default '0',
  `retest_hbins` varchar(255) default NULL,
  `rework_code` tinyint(3) unsigned NOT NULL default '0',
  `job_nam` varchar(255) NOT NULL default '',
  `job_rev` varchar(255) NOT NULL default '',
  `oper_nam` varchar(255) NOT NULL default '',
  `exec_typ` varchar(255) NOT NULL default '',
  `exec_ver` varchar(255) NOT NULL default '',
  `test_cod` varchar(255) NOT NULL default '',
  `facil_id` varchar(255) NOT NULL default '',
  `tst_temp` varchar(255) NOT NULL default '',
  `nb_sites` tinyint(3) unsigned NOT NULL default '1',
  `head_num` tinyint(3) unsigned default NULL,
  `handler_id` varchar(255) default NULL,
  `loadboard_id` varchar(255) default NULL,
  `dib_id` varchar(255) default NULL,
  `card_id` varchar(255) default NULL,
  `file_host_id` int(10) unsigned default '0',
  `file_path` varchar(255) NOT NULL default '',
  `file_name` varchar(255) NOT NULL default '',
  PRIMARY KEY  (`splitlot_id`),
  UNIQUE KEY `uk_et_splitlot_1` (`lot_id`,`tester_name`,`stat_num`,`start_t`,`wafer_id`),
  KEY `fk_et_splitlot_2` (`file_host_id`),
  CONSTRAINT `fk_et_splitlot_1` FOREIGN KEY (`lot_id`) REFERENCES `et_lot` (`lot_id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_et_splitlot_2` FOREIGN KEY (`file_host_id`) REFERENCES `file_host` (`file_host_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40000 ALTER TABLE `et_splitlot` DISABLE KEYS */;
/*!40000 ALTER TABLE `et_splitlot` ENABLE KEYS */;


DROP TABLE IF EXISTS `et_stats_hbin`;
CREATE TABLE `et_stats_hbin` (
  `splitlot_id` int(10) unsigned NOT NULL default '0',
  `site_no` smallint(5) unsigned NOT NULL default '255',
  `hbin_id` int(10) unsigned NOT NULL default '0',
  `nb_parts` int(10) unsigned NOT NULL default '0',
  `nb_runs` int(10) unsigned NOT NULL default '0',
  UNIQUE KEY `uk_et_stats_hbin` (`splitlot_id`,`site_no`,`hbin_id`),
  KEY `fk_et_stats_hbin_2` (`hbin_id`),
  CONSTRAINT `fk_et_stats_hbin_1` FOREIGN KEY (`splitlot_id`) REFERENCES `et_splitlot` (`splitlot_id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_et_stats_hbin_2` FOREIGN KEY (`hbin_id`) REFERENCES `et_hbin` (`hbin_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40000 ALTER TABLE `et_stats_hbin` DISABLE KEYS */;
/*!40000 ALTER TABLE `et_stats_hbin` ENABLE KEYS */;


DROP TABLE IF EXISTS `et_stats_parts`;
CREATE TABLE `et_stats_parts` (
  `splitlot_id` int(10) unsigned NOT NULL default '0',
  `site_no` smallint(5) unsigned NOT NULL default '255',
  `nb_runs` int(10) unsigned NOT NULL default '0',
  `nb_runs_good` int(10) unsigned NOT NULL default '0',
  UNIQUE KEY `uk_et_stats_parts` (`splitlot_id`,`site_no`),
  CONSTRAINT `fk_et_stats_parts_1` FOREIGN KEY (`splitlot_id`) REFERENCES `et_splitlot` (`splitlot_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40000 ALTER TABLE `et_stats_parts` DISABLE KEYS */;
/*!40000 ALTER TABLE `et_stats_parts` ENABLE KEYS */;


DROP TABLE IF EXISTS `et_stats_samples_ptest`;
CREATE TABLE `et_stats_samples_ptest` (
  `ptest_info_id` int(10) unsigned NOT NULL default '0',
  `site_no` 	smallint(5) unsigned NOT NULL default '0',
  `splitlot_id` int(10) unsigned NOT NULL default '0',
  `exec_count` 	int(10) unsigned NOT NULL default '0',
  `fail_count` 	int(10) unsigned default NULL,
  `min_value` 	float default NULL,
  `max_value` 	float default NULL,
  `sum` 		float default NULL,
  `square_sum` 	float default NULL,
  UNIQUE KEY `uk_et_stats_samples_ptest_1` (`ptest_info_id`,`site_no`,`splitlot_id`),
  KEY `fk_et_stats_samples_ptest_2` (`splitlot_id`),
  CONSTRAINT `fk_et_stats_samples_ptest_1` FOREIGN KEY (`ptest_info_id`) REFERENCES `et_ptest_info` (`ptest_info_id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_et_stats_samples_ptest_2` FOREIGN KEY (`splitlot_id`) REFERENCES `et_splitlot` (`splitlot_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40000 ALTER TABLE `et_stats_samples_ptest` DISABLE KEYS */;
/*!40000 ALTER TABLE `et_stats_samples_ptest` ENABLE KEYS */;


DROP TABLE IF EXISTS `et_stats_sbin`;
CREATE TABLE `et_stats_sbin` (
  `splitlot_id` int(10) unsigned NOT NULL default '0',
  `site_no` smallint(5) unsigned NOT NULL default '255',
  `sbin_id` int(10) unsigned NOT NULL default '0',
  `nb_parts` int(10) unsigned NOT NULL default '0',
  `nb_runs` int(10) unsigned NOT NULL default '0',
  UNIQUE KEY `uk_et_stats_sbin_1` (`splitlot_id`,`site_no`,`sbin_id`),
  KEY `fk_et_stats_sbin_2` (`sbin_id`),
  CONSTRAINT `fk_et_stats_sbin_1` FOREIGN KEY (`splitlot_id`) REFERENCES `et_splitlot` (`splitlot_id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_et_stats_sbin_2` FOREIGN KEY (`sbin_id`) REFERENCES `et_sbin` (`sbin_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40000 ALTER TABLE `et_stats_sbin` DISABLE KEYS */;
/*!40000 ALTER TABLE `et_stats_sbin` ENABLE KEYS */;


DROP TABLE IF EXISTS `et_stats_summary_ptest`;
CREATE TABLE `et_stats_summary_ptest` (
  `ptest_info_id` int(10) unsigned NOT NULL default '0',
  `site_no` smallint(5) unsigned NOT NULL default '0',
  `splitlot_id` int(10) unsigned NOT NULL default '0',
  `exec_count` int(10) unsigned default NULL,
  `fail_count` int(10) unsigned default NULL,
  `min_value` float default NULL,
  `max_value` float default NULL,
  `sum` float default NULL,
  `square_sum` float default NULL,
  `ttime` int(10) unsigned default NULL,
  UNIQUE KEY `uk_et_stats_summary_ptest_1` (`ptest_info_id`,`site_no`,`splitlot_id`),
  KEY `fk_et_stats_summary_ptest_2` (`splitlot_id`),
  CONSTRAINT `fk_et_stats_summary_ptest_1` FOREIGN KEY (`ptest_info_id`) REFERENCES `et_ptest_info` (`ptest_info_id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_et_stats_summary_ptest_2` FOREIGN KEY (`splitlot_id`) REFERENCES `et_splitlot` (`splitlot_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40000 ALTER TABLE `et_stats_summary_ptest` DISABLE KEYS */;
/*!40000 ALTER TABLE `et_stats_summary_ptest` ENABLE KEYS */;


DROP TABLE IF EXISTS `et_wafer_info`;
CREATE TABLE `et_wafer_info` (
  `splitlot_id` int(10) unsigned NOT NULL default '0',
  `wafer_id` varchar(255) default NULL,
  `fab_id` varchar(255) default NULL,
  `frame_id` varchar(255) default NULL,
  `mask_id` varchar(255) default NULL,
  `wafer_size` float default NULL,
  `die_ht` float default NULL,
  `die_wid` float default NULL,
  `wafer_units` tinyint(3) unsigned default NULL,
  `wafer_flat` char(1) default NULL,
  `center_x` smallint(5) unsigned default NULL,
  `center_y` smallint(5) unsigned default NULL,
  `pos_x` char(1) default NULL,
  `pos_y` char(1) default NULL,
  KEY `fk_et_wafer_info_1` (`splitlot_id`),
  CONSTRAINT `fk_et_wafer_info_1` FOREIGN KEY (`splitlot_id`) REFERENCES `et_splitlot` (`splitlot_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40000 ALTER TABLE `et_wafer_info` DISABLE KEYS */;
/*!40000 ALTER TABLE `et_wafer_info` ENABLE KEYS */;


DROP TABLE IF EXISTS `file_host`;
CREATE TABLE `file_host` (
  `file_host_id` int(10) unsigned NOT NULL auto_increment,
  `host_name` varchar(255) NOT NULL default '',
  `host_ftpuser` varchar(255) NOT NULL default '',
  `host_ftppassword` varchar(255) NOT NULL default '',
  `host_ftppath` varchar(255) default NULL,
  PRIMARY KEY  (`file_host_id`),
  UNIQUE KEY `uk_file_host_1` (`host_name`,`host_ftpuser`,`host_ftppath`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40000 ALTER TABLE `file_host` DISABLE KEYS */;
/*!40000 ALTER TABLE `file_host` ENABLE KEYS */;


DROP TABLE IF EXISTS `ft_ftest_info`;
CREATE TABLE `ft_ftest_info` (
  `ftest_info_id` int(10) unsigned NOT NULL auto_increment,
  `splitlot_id` int(10) unsigned NOT NULL default '0',
  `tnum` int(10) unsigned NOT NULL default '0',
  `tname` varchar(255) NOT NULL default '',
  `testseq` smallint(5) unsigned default NULL,
  PRIMARY KEY  (`ftest_info_id`),
  UNIQUE KEY `uk_ft_ftest_info_1` (`splitlot_id`,`tnum`,`tname`),
  CONSTRAINT `fk_ft_ftest_info_1` FOREIGN KEY (`splitlot_id`) REFERENCES `ft_splitlot` (`splitlot_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40000 ALTER TABLE `ft_ftest_info` DISABLE KEYS */;
/*!40000 ALTER TABLE `ft_ftest_info` ENABLE KEYS */;


DROP TABLE IF EXISTS `ft_hbin`;
CREATE TABLE `ft_hbin` (
  `hbin_id` int(10) unsigned NOT NULL auto_increment,
  `splitlot_id` int(10) unsigned NOT NULL default '0',
  `hbin_no` smallint(5) unsigned NOT NULL default '0',
  `hbin_name` varchar(255) NOT NULL default '',
  `hbin_cat` char(1) default NULL,
  `nb_parts` int(10) unsigned NOT NULL default '0',
  `nb_runs` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`hbin_id`),
  UNIQUE KEY `uk_ft_hbin_1` (`splitlot_id`,`hbin_no`),
  CONSTRAINT `fk_ft_hbin_1` FOREIGN KEY (`splitlot_id`) REFERENCES `ft_splitlot` (`splitlot_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40000 ALTER TABLE `ft_hbin` DISABLE KEYS */;
/*!40000 ALTER TABLE `ft_hbin` ENABLE KEYS */;


DROP TABLE IF EXISTS `ft_lot`;
CREATE TABLE `ft_lot` (
  `lot_id` varchar(255) NOT NULL default '',
  `product_name` varchar(255) default NULL,
  `nb_parts` int(10) unsigned NOT NULL default '0',
  `nb_parts_good` int(10) unsigned NOT NULL default '0',
  `nb_runs` int(10) unsigned NOT NULL default '0',
  `nb_runs_good` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`lot_id`),
  KEY `fk_ft_lot_1` (`product_name`),
  CONSTRAINT `fk_ft_lot_1` FOREIGN KEY (`product_name`) REFERENCES `product` (`product_name`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40000 ALTER TABLE `ft_lot` DISABLE KEYS */;
/*!40000 ALTER TABLE `ft_lot` ENABLE KEYS */;


DROP TABLE IF EXISTS `ft_mptest_info`;
CREATE TABLE `ft_mptest_info` (
  `mptest_info_id` int(10) unsigned NOT NULL auto_increment,
  `splitlot_id` int(10) unsigned NOT NULL default '0',
  `tnum` int(10) unsigned NOT NULL default '0',
  `tname` varchar(255) NOT NULL default '',
  `tpin` smallint(6) NOT NULL default '0',
  `units` varchar(255) NOT NULL default '',
  `flags` binary(1) NOT NULL default '\0',
  `ll` float default NULL,
  `hl` float default NULL,
  `testseq` smallint(5) unsigned default NULL,
  PRIMARY KEY  (`mptest_info_id`),
  UNIQUE KEY `uk_ft_mptest_info_1` (`splitlot_id`,`tnum`,`tname`,`tpin`),
  CONSTRAINT `fk_ft_mptest_info_1` FOREIGN KEY (`splitlot_id`) REFERENCES `ft_splitlot` (`splitlot_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40000 ALTER TABLE `ft_mptest_info` DISABLE KEYS */;
/*!40000 ALTER TABLE `ft_mptest_info` ENABLE KEYS */;


DROP TABLE IF EXISTS `ft_mptest_limits`;
CREATE TABLE `ft_mptest_limits` (
  `mptest_info_id` int(10) unsigned NOT NULL default '0',
  `site_no` smallint(5) unsigned NOT NULL default '255',
  `splitlot_id` int(10) unsigned NOT NULL default '0',
  `ll` float NOT NULL default '0',
  `hl` float NOT NULL default '0',
  UNIQUE KEY `uk_ft_mptest_limits_1` (`mptest_info_id`,`site_no`,`splitlot_id`),
  KEY `fk_ft_mptest_limits_2` (`splitlot_id`),
  CONSTRAINT `fk_ft_mptest_limits_1` FOREIGN KEY (`mptest_info_id`) REFERENCES `ft_mptest_info` (`mptest_info_id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_ft_mptest_limits_2` FOREIGN KEY (`splitlot_id`) REFERENCES `ft_splitlot` (`splitlot_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40000 ALTER TABLE `ft_mptest_limits` DISABLE KEYS */;
/*!40000 ALTER TABLE `ft_mptest_limits` ENABLE KEYS */;


DROP TABLE IF EXISTS `ft_ptest_info`;
CREATE TABLE `ft_ptest_info` (
  `ptest_info_id` int(10) unsigned NOT NULL auto_increment,
  `splitlot_id` int(10) unsigned NOT NULL default '0',
  `tnum` int(10) unsigned NOT NULL default '0',
  `tname` varchar(255) NOT NULL default '',
  `units` varchar(255) NOT NULL default '',
  `flags` binary(1) NOT NULL default '\0',
  `ll` float default NULL,
  `hl` float default NULL,
  `testseq` smallint(5) unsigned default NULL,
  PRIMARY KEY  (`ptest_info_id`),
  UNIQUE KEY `uk_ft_ptest_info_1` (`splitlot_id`,`tnum`,`tname`),
  CONSTRAINT `fk_ft_ptest_info_1` FOREIGN KEY (`splitlot_id`) REFERENCES `ft_splitlot` (`splitlot_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40000 ALTER TABLE `ft_ptest_info` DISABLE KEYS */;
/*!40000 ALTER TABLE `ft_ptest_info` ENABLE KEYS */;


DROP TABLE IF EXISTS `ft_ptest_limits`;
CREATE TABLE `ft_ptest_limits` (
  `ptest_info_id` int(10) unsigned NOT NULL default '0',
  `site_no` smallint(5) unsigned NOT NULL default '255',
  `splitlot_id` int(10) unsigned NOT NULL default '0',
  `ll` float NOT NULL default '0',
  `hl` float NOT NULL default '0',
  UNIQUE KEY `uk_ft_ptest_limits_1` (`ptest_info_id`,`site_no`,`splitlot_id`),
  KEY `fk_ft_ptest_limits_2` (`splitlot_id`),
  CONSTRAINT `fk_ft_ptest_limits_1` FOREIGN KEY (`ptest_info_id`) REFERENCES `ft_ptest_info` (`ptest_info_id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_ft_ptest_limits_2` FOREIGN KEY (`splitlot_id`) REFERENCES `ft_splitlot` (`splitlot_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40000 ALTER TABLE `ft_ptest_limits` DISABLE KEYS */;
/*!40000 ALTER TABLE `ft_ptest_limits` ENABLE KEYS */;


DROP TABLE IF EXISTS `ft_result_ftest`;
CREATE TABLE `ft_result_ftest` (
  `ftest_info_id` int(10) unsigned NOT NULL,
  `run_id` int(10) unsigned NOT NULL default '255',
  `flags` binary(1) NOT NULL default '\0',
  `vect_nam` varchar(255) NOT NULL default '',
  `vect_off` smallint(6) default NULL,
  KEY `fk_ft_result_ftest_1` (`ftest_info_id`),
  KEY `fk_ft_result_ftest_2` (`run_id`),
  CONSTRAINT `fk_ft_result_ftest_1` FOREIGN KEY (`ftest_info_id`) REFERENCES `ft_ftest_info` (`ftest_info_id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_ft_result_ftest_2` FOREIGN KEY (`run_id`) REFERENCES `ft_run` (`run_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40000 ALTER TABLE `ft_result_ftest` DISABLE KEYS */;
/*!40000 ALTER TABLE `ft_result_ftest` ENABLE KEYS */;


DROP TABLE IF EXISTS `ft_result_mptest`;
CREATE TABLE `ft_result_mptest` (
  `mptest_info_id` int(10) unsigned NOT NULL default '0',
  `run_id` int(10) unsigned NOT NULL default '255',
  `flags` binary(1) NOT NULL default '\0',
  `value` float NOT NULL default '0',
  KEY `fk_ft_result_mptest_1` (`mptest_info_id`),
  KEY `fk_ft_result_mptest_2` (`run_id`),
  CONSTRAINT `fk_ft_result_mptest_1` FOREIGN KEY (`mptest_info_id`) REFERENCES `ft_mptest_info` (`mptest_info_id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_ft_result_mptest_2` FOREIGN KEY (`run_id`) REFERENCES `ft_run` (`run_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40000 ALTER TABLE `ft_result_mptest` DISABLE KEYS */;
/*!40000 ALTER TABLE `ft_result_mptest` ENABLE KEYS */;


DROP TABLE IF EXISTS `ft_result_ptest`;
CREATE TABLE `ft_result_ptest` (
  `ptest_info_id` int(10) unsigned NOT NULL default '0',
  `run_id` int(10) unsigned NOT NULL default '255',
  `flags` binary(1) NOT NULL default '\0',
  `value` float default NULL,
  KEY `fk_ft_result_ptest_1` (`ptest_info_id`),
  KEY `fk_ft_result_ptest_2` (`run_id`),
  CONSTRAINT `fk_ft_result_ptest_1` FOREIGN KEY (`ptest_info_id`) REFERENCES `ft_ptest_info` (`ptest_info_id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_ft_result_ptest_2` FOREIGN KEY (`run_id`) REFERENCES `ft_run` (`run_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40000 ALTER TABLE `ft_result_ptest` DISABLE KEYS */;
/*!40000 ALTER TABLE `ft_result_ptest` ENABLE KEYS */;


DROP TABLE IF EXISTS `ft_run`;
CREATE TABLE `ft_run` (
  `run_id` int(10) unsigned NOT NULL auto_increment,
  `splitlot_id` int(10) unsigned NOT NULL default '0',
  `site_no` smallint(5) unsigned NOT NULL default '255',
  `part_id` varchar(255) default NULL,
  `part_x` smallint(6) default NULL,
  `part_y` smallint(6) default NULL,
  `hbin_no` smallint(5) unsigned NOT NULL default '0',
  `sbin_no` smallint(5) unsigned default NULL,
  `ttime` int(10) unsigned default NULL,
  `tests_executed` smallint(5) unsigned NOT NULL default '0',
  `tests_failed` smallint(5) unsigned NOT NULL default '0',
  `firstfail_tnum` int(10) unsigned default NULL,
  `firstfail_tname` varchar(255) default NULL,
  `retest_index` tinyint(3) unsigned NOT NULL default '0',
  PRIMARY KEY  (`run_id`),
  KEY `fk_ft_run_1` (`splitlot_id`),
  CONSTRAINT `fk_ft_run_1` FOREIGN KEY (`splitlot_id`) REFERENCES `ft_splitlot` (`splitlot_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40000 ALTER TABLE `ft_run` DISABLE KEYS */;
/*!40000 ALTER TABLE `ft_run` ENABLE KEYS */;


DROP TABLE IF EXISTS `ft_sbin`;
CREATE TABLE `ft_sbin` (
  `sbin_id` int(10) unsigned NOT NULL auto_increment,
  `splitlot_id` int(10) unsigned NOT NULL default '0',
  `sbin_no` smallint(5) unsigned NOT NULL default '0',
  `sbin_name` varchar(255) NOT NULL default '',
  `sbin_cat` char(1) default NULL,
  `nb_parts` int(10) unsigned NOT NULL default '0',
  `nb_runs` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`sbin_id`),
  UNIQUE KEY `uk_ft_sbin_1` (`splitlot_id`,`sbin_no`),
  CONSTRAINT `fk_ft_sbin_1` FOREIGN KEY (`splitlot_id`) REFERENCES `ft_splitlot` (`splitlot_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40000 ALTER TABLE `ft_sbin` DISABLE KEYS */;
/*!40000 ALTER TABLE `ft_sbin` ENABLE KEYS */;


DROP TABLE IF EXISTS `ft_splitlot`;
CREATE TABLE `ft_splitlot` (
  `splitlot_id` int(10) unsigned NOT NULL auto_increment,
  `lot_id` varchar(255) NOT NULL default '0',
  `sublot_id` varchar(255) NOT NULL default '',
  `setup_t` int(10) unsigned NOT NULL default '0',
  `start_t` int(10) unsigned NOT NULL default '0',
  `finish_t` int(10) unsigned NOT NULL default '0',
  `stat_num` tinyint(3) unsigned NOT NULL default '0',
  `tester_name` varchar(255) NOT NULL default '',
  `flags` binary(2) NOT NULL default '\0\0',
  `nb_parts` int(10) unsigned NOT NULL default '0',
  `nb_parts_good` int(10) unsigned NOT NULL default '0',
  `nb_runs` int(10) unsigned NOT NULL default '0',
  `nb_runs_good` int(10) unsigned NOT NULL default '0',
  `data_origin` tinyint(3) unsigned NOT NULL default '0',
  `retest_index` tinyint(3) unsigned NOT NULL default '0',
  `retest_hbins` varchar(255) default NULL,
  `rework_code` tinyint(3) unsigned NOT NULL default '0',
  `job_nam` varchar(255) NOT NULL default '',
  `job_rev` varchar(255) NOT NULL default '',
  `oper_nam` varchar(255) NOT NULL default '',
  `exec_typ` varchar(255) NOT NULL default '',
  `exec_ver` varchar(255) NOT NULL default '',
  `test_cod` varchar(255) NOT NULL default '',
  `facil_id` varchar(255) NOT NULL default '',
  `tst_temp` varchar(255) NOT NULL default '',
  `nb_sites` tinyint(3) unsigned NOT NULL default '1',
  `head_num` tinyint(3) unsigned default NULL,
  `handler_id` varchar(255) default NULL,
  `loadboard_id` varchar(255) default NULL,
  `dib_id` varchar(255) default NULL,
  `card_id` varchar(255) default NULL,
  `file_host_id` int(10) unsigned default '0',
  `file_path` varchar(255) NOT NULL default '',
  `file_name` varchar(255) NOT NULL default '',
  PRIMARY KEY  (`splitlot_id`),
  UNIQUE KEY `uk_ft_splitlot_1` (`lot_id`,`tester_name`,`stat_num`,`start_t`),
  KEY `fk_ft_splitlot_2` (`file_host_id`),
  CONSTRAINT `fk_ft_splitlot_1` FOREIGN KEY (`lot_id`) REFERENCES `ft_lot` (`lot_id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_ft_splitlot_2` FOREIGN KEY (`file_host_id`) REFERENCES `file_host` (`file_host_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40000 ALTER TABLE `ft_splitlot` DISABLE KEYS */;
/*!40000 ALTER TABLE `ft_splitlot` ENABLE KEYS */;


DROP TABLE IF EXISTS `ft_stats_hbin`;
CREATE TABLE `ft_stats_hbin` (
  `splitlot_id` int(10) unsigned NOT NULL default '0',
  `site_no` smallint(5) unsigned NOT NULL default '255',
  `hbin_id` int(10) unsigned NOT NULL default '0',
  `nb_parts` int(10) unsigned NOT NULL default '0',
  `nb_runs` int(10) unsigned NOT NULL default '0',
  UNIQUE KEY `uk_ft_stats_hbin` (`splitlot_id`,`site_no`,`hbin_id`),
  KEY `fk_ft_stats_hbin_2` (`hbin_id`),
  CONSTRAINT `fk_ft_stats_hbin_1` FOREIGN KEY (`splitlot_id`) REFERENCES `ft_splitlot` (`splitlot_id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_ft_stats_hbin_2` FOREIGN KEY (`hbin_id`) REFERENCES `ft_hbin` (`hbin_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40000 ALTER TABLE `ft_stats_hbin` DISABLE KEYS */;
/*!40000 ALTER TABLE `ft_stats_hbin` ENABLE KEYS */;


DROP TABLE IF EXISTS `ft_stats_parts`;
CREATE TABLE `ft_stats_parts` (
  `splitlot_id` int(10) unsigned NOT NULL default '0',
  `site_no` smallint(5) unsigned NOT NULL default '255',
  `nb_runs` int(10) unsigned NOT NULL default '0',
  `nb_runs_good` int(10) unsigned NOT NULL default '0',
  UNIQUE KEY `uk_ft_stats_parts` (`splitlot_id`,`site_no`),
  CONSTRAINT `fk_ft_stats_parts_1` FOREIGN KEY (`splitlot_id`) REFERENCES `ft_splitlot` (`splitlot_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40000 ALTER TABLE `ft_stats_parts` DISABLE KEYS */;
/*!40000 ALTER TABLE `ft_stats_parts` ENABLE KEYS */;


DROP TABLE IF EXISTS `ft_stats_samples_ftest`;
CREATE TABLE `ft_stats_samples_ftest` (
  `ftest_info_id` int(10) unsigned NOT NULL default '0',
  `site_no` smallint(5) unsigned NOT NULL default '255',
  `splitlot_id` int(10) unsigned NOT NULL default '0',
  `exec_count` int(10) unsigned NOT NULL default '0',
  `fail_count` int(10) unsigned default NULL,
  UNIQUE KEY `uk_ft_stats_samples_ftest_1` (`ftest_info_id`,`site_no`,`splitlot_id`),
  KEY `fk_ft_stats_samples_ftest_2` (`splitlot_id`),
  CONSTRAINT `fk_ft_stats_samples_ftest_1` FOREIGN KEY (`ftest_info_id`) REFERENCES `ft_ftest_info` (`ftest_info_id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_ft_stats_samples_ftest_2` FOREIGN KEY (`splitlot_id`) REFERENCES `ft_splitlot` (`splitlot_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40000 ALTER TABLE `ft_stats_samples_ftest` DISABLE KEYS */;
/*!40000 ALTER TABLE `ft_stats_samples_ftest` ENABLE KEYS */;


DROP TABLE IF EXISTS `ft_stats_samples_mptest`;
CREATE TABLE `ft_stats_samples_mptest` (
  `mptest_info_id` int(10) unsigned NOT NULL default '0',
  `site_no` smallint(5) unsigned NOT NULL default '255',
  `splitlot_id` int(10) unsigned NOT NULL default '0',
  `exec_count` int(10) unsigned NOT NULL default '0',
  `fail_count` int(10) unsigned default NULL,
  `min_value` float default NULL,
  `max_value` float default NULL,
  `sum` float default NULL,
  `square_sum` float default NULL,
  UNIQUE KEY `uk_ft_stats_samples_mptest_1` (`mptest_info_id`,`site_no`,`splitlot_id`),
  KEY `fk_ft_stats_samples_mptest_2` (`splitlot_id`),
  CONSTRAINT `fk_ft_stats_samples_mptest_1` FOREIGN KEY (`mptest_info_id`) REFERENCES `ft_mptest_info` (`mptest_info_id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_ft_stats_samples_mptest_2` FOREIGN KEY (`splitlot_id`) REFERENCES `ft_splitlot` (`splitlot_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40000 ALTER TABLE `ft_stats_samples_mptest` DISABLE KEYS */;
/*!40000 ALTER TABLE `ft_stats_samples_mptest` ENABLE KEYS */;


DROP TABLE IF EXISTS `ft_stats_samples_ptest`;
CREATE TABLE `ft_stats_samples_ptest` (
  `ptest_info_id` int(10) unsigned NOT NULL default '0',
  `site_no` smallint(5) unsigned NOT NULL default '0',
  `splitlot_id` int(10) unsigned NOT NULL default '0',
  `exec_count` int(10) unsigned NOT NULL default '0',
  `fail_count` int(10) unsigned default NULL,
  `min_value` float default NULL,
  `max_value` float default NULL,
  `sum` float default NULL,
  `square_sum` float default NULL,
  UNIQUE KEY `uk_ft_stats_samples_ptest_1` (`ptest_info_id`,`site_no`,`splitlot_id`),
  KEY `fk_ft_stats_samples_ptest_2` (`splitlot_id`),
  CONSTRAINT `fk_ft_stats_samples_ptest_1` FOREIGN KEY (`ptest_info_id`) REFERENCES `ft_ptest_info` (`ptest_info_id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_ft_stats_samples_ptest_2` FOREIGN KEY (`splitlot_id`) REFERENCES `ft_splitlot` (`splitlot_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40000 ALTER TABLE `ft_stats_samples_ptest` DISABLE KEYS */;
/*!40000 ALTER TABLE `ft_stats_samples_ptest` ENABLE KEYS */;


DROP TABLE IF EXISTS `ft_stats_sbin`;
CREATE TABLE `ft_stats_sbin` (
  `splitlot_id` int(10) unsigned NOT NULL default '0',
  `site_no` smallint(5) unsigned NOT NULL default '255',
  `sbin_id` int(10) unsigned NOT NULL default '0',
  `nb_parts` int(10) unsigned NOT NULL default '0',
  `nb_runs` int(10) unsigned NOT NULL default '0',
  UNIQUE KEY `uk_ft_stats_sbin_1` (`splitlot_id`,`site_no`,`sbin_id`),
  KEY `fk_ft_stats_sbin_2` (`sbin_id`),
  CONSTRAINT `fk_ft_stats_sbin_1` FOREIGN KEY (`splitlot_id`) REFERENCES `ft_splitlot` (`splitlot_id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_ft_stats_sbin_2` FOREIGN KEY (`sbin_id`) REFERENCES `ft_sbin` (`sbin_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40000 ALTER TABLE `ft_stats_sbin` DISABLE KEYS */;
/*!40000 ALTER TABLE `ft_stats_sbin` ENABLE KEYS */;


DROP TABLE IF EXISTS `ft_stats_summary_ftest`;
CREATE TABLE `ft_stats_summary_ftest` (
  `ftest_info_id` int(10) unsigned NOT NULL default '0',
  `site_no` smallint(5) unsigned NOT NULL default '255',
  `splitlot_id` int(10) unsigned NOT NULL default '0',
  `exec_count` int(10) unsigned default NULL,
  `fail_count` int(10) unsigned default NULL,
  `ttime` int(10) unsigned default NULL,
  UNIQUE KEY `uk_ft_stats_summary_ftest_1` (`ftest_info_id`,`site_no`,`splitlot_id`),
  KEY `fk_ft_stats_summary_ftest_2` (`splitlot_id`),
  CONSTRAINT `fk_ft_stats_summary_ftest_1` FOREIGN KEY (`ftest_info_id`) REFERENCES `ft_ftest_info` (`ftest_info_id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_ft_stats_summary_ftest_2` FOREIGN KEY (`splitlot_id`) REFERENCES `ft_splitlot` (`splitlot_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40000 ALTER TABLE `ft_stats_summary_ftest` DISABLE KEYS */;
/*!40000 ALTER TABLE `ft_stats_summary_ftest` ENABLE KEYS */;


DROP TABLE IF EXISTS `ft_stats_summary_mptest`;
CREATE TABLE `ft_stats_summary_mptest` (
  `mptest_info_id` int(10) unsigned NOT NULL default '0',
  `site_no` smallint(5) unsigned NOT NULL default '0',
  `splitlot_id` int(10) unsigned NOT NULL default '0',
  `exec_count` int(10) unsigned NOT NULL default '0',
  `fail_count` int(10) unsigned NOT NULL default '0',
  `min_value` float default NULL,
  `max_value` float default NULL,
  `sum` float default NULL,
  `square_sum` float default NULL,
  `ttime` int(10) unsigned default NULL,
  UNIQUE KEY `uk_ft_stats_summary_mptest_1` (`mptest_info_id`,`site_no`,`splitlot_id`),
  KEY `fk_ft_stats_summary_mptest_2` (`splitlot_id`),
  CONSTRAINT `fk_ft_stats_summary_mptest_1` FOREIGN KEY (`mptest_info_id`) REFERENCES `ft_mptest_info` (`mptest_info_id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_ft_stats_summary_mptest_2` FOREIGN KEY (`splitlot_id`) REFERENCES `ft_splitlot` (`splitlot_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40000 ALTER TABLE `ft_stats_summary_mptest` DISABLE KEYS */;
/*!40000 ALTER TABLE `ft_stats_summary_mptest` ENABLE KEYS */;


DROP TABLE IF EXISTS `ft_stats_summary_ptest`;
CREATE TABLE `ft_stats_summary_ptest` (
  `ptest_info_id` int(10) unsigned NOT NULL default '0',
  `site_no` smallint(5) unsigned NOT NULL default '0',
  `splitlot_id` int(10) unsigned NOT NULL default '0',
  `exec_count` int(10) unsigned default NULL,
  `fail_count` int(10) unsigned default NULL,
  `min_value` float default NULL,
  `max_value` float default NULL,
  `sum` float default NULL,
  `square_sum` float default NULL,
  `ttime` int(10) unsigned default NULL,
  UNIQUE KEY `uk_ft_stats_summary_ptest_1` (`ptest_info_id`,`site_no`,`splitlot_id`),
  KEY `fk_ft_stats_summary_ptest_2` (`splitlot_id`),
  CONSTRAINT `fk_ft_stats_summary_ptest_1` FOREIGN KEY (`ptest_info_id`) REFERENCES `ft_ptest_info` (`ptest_info_id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_ft_stats_summary_ptest_2` FOREIGN KEY (`splitlot_id`) REFERENCES `ft_splitlot` (`splitlot_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40000 ALTER TABLE `ft_stats_summary_ptest` DISABLE KEYS */;
/*!40000 ALTER TABLE `ft_stats_summary_ptest` ENABLE KEYS */;


DROP TABLE IF EXISTS `product`;
CREATE TABLE `product` (
  `product_name` varchar(255) NOT NULL default '',
  `description` varchar(1000) default NULL,
  PRIMARY KEY  (`product_name`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40000 ALTER TABLE `product` DISABLE KEYS */;
/*!40000 ALTER TABLE `product` ENABLE KEYS */;


DROP TABLE IF EXISTS `wt_ftest_info`;
CREATE TABLE `wt_ftest_info` (
  `ftest_info_id` int(10) unsigned NOT NULL auto_increment,
  `splitlot_id` int(10) unsigned NOT NULL default '0',
  `tnum` int(10) unsigned NOT NULL default '0',
  `tname` varchar(255) NOT NULL default '',
  `testseq` smallint(5) unsigned default NULL,
  PRIMARY KEY  (`ftest_info_id`),
  UNIQUE KEY `uk_wt_ftest_info_1` (`splitlot_id`,`tnum`,`tname`),
  CONSTRAINT `fk_wt_ftest_info_1` FOREIGN KEY (`splitlot_id`) REFERENCES `wt_splitlot` (`splitlot_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40000 ALTER TABLE `wt_ftest_info` DISABLE KEYS */;
/*!40000 ALTER TABLE `wt_ftest_info` ENABLE KEYS */;


DROP TABLE IF EXISTS `wt_hbin`;
CREATE TABLE `wt_hbin` (
  `hbin_id` int(10) unsigned NOT NULL auto_increment,
  `splitlot_id` int(10) unsigned NOT NULL default '0',
  `hbin_no` smallint(5) unsigned NOT NULL default '0',
  `hbin_name` varchar(255) NOT NULL default '',
  `hbin_cat` char(1) default NULL,
  `nb_parts` int(10) unsigned NOT NULL default '0',
  `nb_runs` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`hbin_id`),
  UNIQUE KEY `uk_wt_hbin_1` (`splitlot_id`,`hbin_no`),
  CONSTRAINT `fk_wt_hbin_1` FOREIGN KEY (`splitlot_id`) REFERENCES `wt_splitlot` (`splitlot_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40000 ALTER TABLE `wt_hbin` DISABLE KEYS */;
/*!40000 ALTER TABLE `wt_hbin` ENABLE KEYS */;


DROP TABLE IF EXISTS `wt_lot`;
CREATE TABLE `wt_lot` (
  `lot_id` varchar(255) NOT NULL default '',
  `product_name` varchar(255) default NULL,
  `nb_parts` int(10) unsigned NOT NULL default '0',
  `nb_parts_good` int(10) unsigned NOT NULL default '0',
  `nb_runs` int(10) unsigned NOT NULL default '0',
  `nb_runs_good` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`lot_id`),
  KEY `fk_wt_lot_1` (`product_name`),
  CONSTRAINT `fk_wt_lot_1` FOREIGN KEY (`product_name`) REFERENCES `product` (`product_name`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40000 ALTER TABLE `wt_lot` DISABLE KEYS */;
/*!40000 ALTER TABLE `wt_lot` ENABLE KEYS */;


DROP TABLE IF EXISTS `wt_mptest_info`;
CREATE TABLE `wt_mptest_info` (
  `mptest_info_id` int(10) unsigned NOT NULL auto_increment,
  `splitlot_id` int(10) unsigned NOT NULL default '0',
  `tnum` int(10) unsigned NOT NULL default '0',
  `tname` varchar(255) NOT NULL default '',
  `tpin` smallint(6) NOT NULL default '0',
  `units` varchar(255) NOT NULL default '',
  `flags` binary(1) NOT NULL default '\0',
  `ll` float default NULL,
  `hl` float default NULL,
  `testseq` smallint(5) unsigned default NULL,
  PRIMARY KEY  (`mptest_info_id`),
  UNIQUE KEY `uk_wt_mptest_info_1` (`splitlot_id`,`tnum`,`tname`,`tpin`),
  CONSTRAINT `fk_wt_mptest_info_1` FOREIGN KEY (`splitlot_id`) REFERENCES `wt_splitlot` (`splitlot_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40000 ALTER TABLE `wt_mptest_info` DISABLE KEYS */;
/*!40000 ALTER TABLE `wt_mptest_info` ENABLE KEYS */;


DROP TABLE IF EXISTS `wt_mptest_limits`;
CREATE TABLE `wt_mptest_limits` (
  `mptest_info_id` int(10) unsigned NOT NULL default '0',
  `site_no` smallint(5) unsigned NOT NULL default '255',
  `splitlot_id` int(10) unsigned NOT NULL default '0',
  `ll` float NOT NULL default '0',
  `hl` float NOT NULL default '0',
  UNIQUE KEY `uk_wt_mptest_limits_1` (`mptest_info_id`,`site_no`,`splitlot_id`),
  KEY `fk_wt_mptest_limits_2` (`splitlot_id`),
  CONSTRAINT `fk_wt_mptest_limits_1` FOREIGN KEY (`mptest_info_id`) REFERENCES `wt_mptest_info` (`mptest_info_id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_wt_mptest_limits_2` FOREIGN KEY (`splitlot_id`) REFERENCES `wt_splitlot` (`splitlot_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40000 ALTER TABLE `wt_mptest_limits` DISABLE KEYS */;
/*!40000 ALTER TABLE `wt_mptest_limits` ENABLE KEYS */;


DROP TABLE IF EXISTS `wt_ptest_info`;
CREATE TABLE `wt_ptest_info` (
  `ptest_info_id` int(10) unsigned NOT NULL auto_increment,
  `splitlot_id` int(10) unsigned NOT NULL default '0',
  `tnum` int(10) unsigned NOT NULL default '0',
  `tname` varchar(255) NOT NULL default '',
  `units` varchar(255) NOT NULL default '',
  `flags` binary(1) NOT NULL default '\0',
  `ll` float default NULL,
  `hl` float default NULL,
  `testseq` smallint(5) unsigned default NULL,
  PRIMARY KEY  (`ptest_info_id`),
  UNIQUE KEY `uk_wt_ptest_info_1` (`splitlot_id`,`tnum`,`tname`),
  CONSTRAINT `fk_wt_ptest_info_1` FOREIGN KEY (`splitlot_id`) REFERENCES `wt_splitlot` (`splitlot_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40000 ALTER TABLE `wt_ptest_info` DISABLE KEYS */;
/*!40000 ALTER TABLE `wt_ptest_info` ENABLE KEYS */;


DROP TABLE IF EXISTS `wt_ptest_limits`;
CREATE TABLE `wt_ptest_limits` (
  `ptest_info_id` int(10) unsigned NOT NULL default '0',
  `site_no` smallint(5) unsigned NOT NULL default '255',
  `splitlot_id` int(10) unsigned NOT NULL default '0',
  `ll` float NOT NULL default '0',
  `hl` float NOT NULL default '0',
  UNIQUE KEY `uk_wt_ptest_limits_1` (`ptest_info_id`,`site_no`,`splitlot_id`),
  KEY `fk_wt_ptest_limits_2` (`splitlot_id`),
  CONSTRAINT `fk_wt_ptest_limits_1` FOREIGN KEY (`ptest_info_id`) REFERENCES `wt_ptest_info` (`ptest_info_id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_wt_ptest_limits_2` FOREIGN KEY (`splitlot_id`) REFERENCES `wt_splitlot` (`splitlot_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40000 ALTER TABLE `wt_ptest_limits` DISABLE KEYS */;
/*!40000 ALTER TABLE `wt_ptest_limits` ENABLE KEYS */;


DROP TABLE IF EXISTS `wt_result_ftest`;
CREATE TABLE `wt_result_ftest` (
  `ftest_info_id` int(10) unsigned NOT NULL,
  `run_id` int(10) unsigned NOT NULL default '255',
  `flags` binary(1) NOT NULL default '\0',
  `vect_nam` varchar(255) NOT NULL default '',
  `vect_off` smallint(6) default NULL,
  KEY `fk_wt_result_ftest_1` (`ftest_info_id`),
  KEY `fk_wt_result_ftest_2` (`run_id`),
  CONSTRAINT `fk_wt_result_ftest_1` FOREIGN KEY (`ftest_info_id`) REFERENCES `wt_ftest_info` (`ftest_info_id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_wt_result_ftest_2` FOREIGN KEY (`run_id`) REFERENCES `wt_run` (`run_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40000 ALTER TABLE `wt_result_ftest` DISABLE KEYS */;
/*!40000 ALTER TABLE `wt_result_ftest` ENABLE KEYS */;


DROP TABLE IF EXISTS `wt_result_mptest`;
CREATE TABLE `wt_result_mptest` (
  `mptest_info_id` int(10) unsigned NOT NULL default '0',
  `run_id` int(10) unsigned NOT NULL default '255',
  `flags` binary(1) NOT NULL default '\0',
  `value` float NOT NULL default '0',
  KEY `fk_wt_result_mptest_1` (`mptest_info_id`),
  KEY `fk_wt_result_mptest_2` (`run_id`),
  CONSTRAINT `fk_wt_result_mptest_1` FOREIGN KEY (`mptest_info_id`) REFERENCES `wt_mptest_info` (`mptest_info_id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_wt_result_mptest_2` FOREIGN KEY (`run_id`) REFERENCES `wt_run` (`run_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40000 ALTER TABLE `wt_result_mptest` DISABLE KEYS */;
/*!40000 ALTER TABLE `wt_result_mptest` ENABLE KEYS */;


DROP TABLE IF EXISTS `wt_result_ptest`;
CREATE TABLE `wt_result_ptest` (
  `ptest_info_id` int(10) unsigned NOT NULL default '0',
  `run_id` int(10) unsigned NOT NULL default '255',
  `flags` binary(1) NOT NULL default '\0',
  `value` float default NULL,
  KEY `fk_wt_result_ptest_1` (`ptest_info_id`),
  KEY `fk_wt_result_ptest_2` (`run_id`),
  CONSTRAINT `fk_wt_result_ptest_1` FOREIGN KEY (`ptest_info_id`) REFERENCES `wt_ptest_info` (`ptest_info_id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_wt_result_ptest_2` FOREIGN KEY (`run_id`) REFERENCES `wt_run` (`run_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40000 ALTER TABLE `wt_result_ptest` DISABLE KEYS */;
/*!40000 ALTER TABLE `wt_result_ptest` ENABLE KEYS */;


DROP TABLE IF EXISTS `wt_run`;
CREATE TABLE `wt_run` (
  `run_id` int(10) unsigned NOT NULL auto_increment,
  `splitlot_id` int(10) unsigned NOT NULL default '0',
  `site_no` smallint(5) unsigned NOT NULL default '255',
  `part_id` varchar(255) default NULL,
  `part_x` smallint(6) default NULL,
  `part_y` smallint(6) default NULL,
  `hbin_no` smallint(5) unsigned NOT NULL default '0',
  `sbin_no` smallint(5) unsigned default NULL,
  `ttime` int(10) unsigned default NULL,
  `tests_executed` smallint(5) unsigned NOT NULL default '0',
  `tests_failed` smallint(5) unsigned NOT NULL default '0',
  `firstfail_tnum` int(10) unsigned default NULL,
  `firstfail_tname` varchar(255) default NULL,
  `retest_index` tinyint(3) unsigned NOT NULL default '0',
  PRIMARY KEY  (`run_id`),
  KEY `fk_wt_run_1` (`splitlot_id`),
  CONSTRAINT `fk_wt_run_1` FOREIGN KEY (`splitlot_id`) REFERENCES `wt_splitlot` (`splitlot_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40000 ALTER TABLE `wt_run` DISABLE KEYS */;
/*!40000 ALTER TABLE `wt_run` ENABLE KEYS */;


DROP TABLE IF EXISTS `wt_sbin`;
CREATE TABLE `wt_sbin` (
  `sbin_id` int(10) unsigned NOT NULL auto_increment,
  `splitlot_id` int(10) unsigned NOT NULL default '0',
  `sbin_no` smallint(5) unsigned NOT NULL default '0',
  `sbin_name` varchar(255) NOT NULL default '',
  `sbin_cat` char(1) default NULL,
  `nb_parts` int(10) unsigned NOT NULL default '0',
  `nb_runs` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`sbin_id`),
  UNIQUE KEY `uk_wt_sbin_1` (`splitlot_id`,`sbin_no`),
  CONSTRAINT `fk_wt_sbin_1` FOREIGN KEY (`splitlot_id`) REFERENCES `wt_splitlot` (`splitlot_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40000 ALTER TABLE `wt_sbin` DISABLE KEYS */;
/*!40000 ALTER TABLE `wt_sbin` ENABLE KEYS */;


DROP TABLE IF EXISTS `wt_splitlot`;
CREATE TABLE `wt_splitlot` (
  `splitlot_id` int(10) unsigned NOT NULL auto_increment,
  `lot_id` varchar(255) NOT NULL default '0',
  `sublot_id` varchar(255) NOT NULL default '',
  `wafer_id` varchar(255) default NULL,
  `setup_t` int(10) unsigned NOT NULL default '0',
  `start_t` int(10) unsigned NOT NULL default '0',
  `finish_t` int(10) unsigned NOT NULL default '0',
  `stat_num` tinyint(3) unsigned NOT NULL default '0',
  `tester_name` varchar(255) NOT NULL default '',
  `flags` binary(2) NOT NULL default '\0\0',
  `nb_parts` int(10) unsigned NOT NULL default '0',
  `nb_parts_good` int(10) unsigned NOT NULL default '0',
  `nb_runs` int(10) unsigned NOT NULL default '0',
  `nb_runs_good` int(10) unsigned NOT NULL default '0',
  `data_origin` tinyint(3) unsigned NOT NULL default '0',
  `retest_index` tinyint(3) unsigned NOT NULL default '0',
  `retest_hbins` varchar(255) default NULL,
  `rework_code` tinyint(3) unsigned NOT NULL default '0',
  `job_nam` varchar(255) NOT NULL default '',
  `job_rev` varchar(255) NOT NULL default '',
  `oper_nam` varchar(255) NOT NULL default '',
  `exec_typ` varchar(255) NOT NULL default '',
  `exec_ver` varchar(255) NOT NULL default '',
  `test_cod` varchar(255) NOT NULL default '',
  `facil_id` varchar(255) NOT NULL default '',
  `tst_temp` varchar(255) NOT NULL default '',
  `nb_sites` tinyint(3) unsigned NOT NULL default '1',
  `head_num` tinyint(3) unsigned default NULL,
  `handler_id` varchar(255) default NULL,
  `loadboard_id` varchar(255) default NULL,
  `dib_id` varchar(255) default NULL,
  `card_id` varchar(255) default NULL,
  `file_host_id` int(10) unsigned default '0',
  `file_path` varchar(255) NOT NULL default '',
  `file_name` varchar(255) NOT NULL default '',
  PRIMARY KEY  (`splitlot_id`),
  UNIQUE KEY `uk_wt_splitlot_1` (`lot_id`,`tester_name`,`stat_num`,`start_t`,`wafer_id`),
  KEY `fk_wt_splitlot_2` (`file_host_id`),
  CONSTRAINT `fk_wt_splitlot_1` FOREIGN KEY (`lot_id`) REFERENCES `wt_lot` (`lot_id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_wt_splitlot_2` FOREIGN KEY (`file_host_id`) REFERENCES `file_host` (`file_host_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40000 ALTER TABLE `wt_splitlot` DISABLE KEYS */;
/*!40000 ALTER TABLE `wt_splitlot` ENABLE KEYS */;


DROP TABLE IF EXISTS `wt_stats_hbin`;
CREATE TABLE `wt_stats_hbin` (
  `splitlot_id` int(10) unsigned NOT NULL default '0',
  `site_no` smallint(5) unsigned NOT NULL default '255',
  `hbin_id` int(10) unsigned NOT NULL default '0',
  `nb_parts` int(10) unsigned NOT NULL default '0',
  `nb_runs` int(10) unsigned NOT NULL default '0',
  UNIQUE KEY `uk_wt_stats_hbin` (`splitlot_id`,`site_no`,`hbin_id`),
  KEY `fk_wt_stats_hbin_2` (`hbin_id`),
  CONSTRAINT `fk_wt_stats_hbin_1` FOREIGN KEY (`splitlot_id`) REFERENCES `wt_splitlot` (`splitlot_id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_wt_stats_hbin_2` FOREIGN KEY (`hbin_id`) REFERENCES `wt_hbin` (`hbin_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40000 ALTER TABLE `wt_stats_hbin` DISABLE KEYS */;
/*!40000 ALTER TABLE `wt_stats_hbin` ENABLE KEYS */;


DROP TABLE IF EXISTS `wt_stats_parts`;
CREATE TABLE `wt_stats_parts` (
  `splitlot_id` int(10) unsigned NOT NULL default '0',
  `site_no` smallint(5) unsigned NOT NULL default '255',
  `nb_runs` int(10) unsigned NOT NULL default '0',
  `nb_runs_good` int(10) unsigned NOT NULL default '0',
  UNIQUE KEY `uk_wt_stats_parts` (`splitlot_id`,`site_no`),
  CONSTRAINT `fk_wt_stats_parts_1` FOREIGN KEY (`splitlot_id`) REFERENCES `wt_splitlot` (`splitlot_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40000 ALTER TABLE `wt_stats_parts` DISABLE KEYS */;
/*!40000 ALTER TABLE `wt_stats_parts` ENABLE KEYS */;


DROP TABLE IF EXISTS `wt_stats_samples_ftest`;
CREATE TABLE `wt_stats_samples_ftest` (
  `ftest_info_id` int(10) unsigned NOT NULL default '0',
  `site_no` smallint(5) unsigned NOT NULL default '255',
  `splitlot_id` int(10) unsigned NOT NULL default '0',
  `exec_count` int(10) unsigned NOT NULL default '0',
  `fail_count` int(10) unsigned default NULL,
  UNIQUE KEY `uk_wt_stats_samples_ftest_1` (`ftest_info_id`,`site_no`,`splitlot_id`),
  KEY `fk_wt_stats_samples_ftest_2` (`splitlot_id`),
  CONSTRAINT `fk_wt_stats_samples_ftest_1` FOREIGN KEY (`ftest_info_id`) REFERENCES `wt_ftest_info` (`ftest_info_id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_wt_stats_samples_ftest_2` FOREIGN KEY (`splitlot_id`) REFERENCES `wt_splitlot` (`splitlot_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40000 ALTER TABLE `wt_stats_samples_ftest` DISABLE KEYS */;
/*!40000 ALTER TABLE `wt_stats_samples_ftest` ENABLE KEYS */;


DROP TABLE IF EXISTS `wt_stats_samples_mptest`;
CREATE TABLE `wt_stats_samples_mptest` (
  `mptest_info_id` int(10) unsigned NOT NULL default '0',
  `site_no` smallint(5) unsigned NOT NULL default '255',
  `splitlot_id` int(10) unsigned NOT NULL default '0',
  `exec_count` int(10) unsigned NOT NULL default '0',
  `fail_count` int(10) unsigned default NULL,
  `min_value` float default NULL,
  `max_value` float default NULL,
  `sum` float default NULL,
  `square_sum` float default NULL,
  UNIQUE KEY `uk_wt_stats_samples_mptest_1` (`mptest_info_id`,`site_no`,`splitlot_id`),
  KEY `fk_wt_stats_samples_mptest_2` (`splitlot_id`),
  CONSTRAINT `fk_wt_stats_samples_mptest_1` FOREIGN KEY (`mptest_info_id`) REFERENCES `wt_mptest_info` (`mptest_info_id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_wt_stats_samples_mptest_2` FOREIGN KEY (`splitlot_id`) REFERENCES `wt_splitlot` (`splitlot_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40000 ALTER TABLE `wt_stats_samples_mptest` DISABLE KEYS */;
/*!40000 ALTER TABLE `wt_stats_samples_mptest` ENABLE KEYS */;


DROP TABLE IF EXISTS `wt_stats_samples_ptest`;
CREATE TABLE `wt_stats_samples_ptest` (
  `ptest_info_id` int(10) unsigned NOT NULL default '0',
  `site_no` smallint(5) unsigned NOT NULL default '0',
  `splitlot_id` int(10) unsigned NOT NULL default '0',
  `exec_count` int(10) unsigned NOT NULL default '0',
  `fail_count` int(10) unsigned default NULL,
  `min_value` float default NULL,
  `max_value` float default NULL,
  `sum` float default NULL,
  `square_sum` float default NULL,
  UNIQUE KEY `uk_wt_stats_samples_ptest_1` (`ptest_info_id`,`site_no`,`splitlot_id`),
  KEY `fk_wt_stats_samples_ptest_2` (`splitlot_id`),
  CONSTRAINT `fk_wt_stats_samples_ptest_1` FOREIGN KEY (`ptest_info_id`) REFERENCES `wt_ptest_info` (`ptest_info_id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_wt_stats_samples_ptest_2` FOREIGN KEY (`splitlot_id`) REFERENCES `wt_splitlot` (`splitlot_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40000 ALTER TABLE `wt_stats_samples_ptest` DISABLE KEYS */;
/*!40000 ALTER TABLE `wt_stats_samples_ptest` ENABLE KEYS */;


DROP TABLE IF EXISTS `wt_stats_sbin`;
CREATE TABLE `wt_stats_sbin` (
  `splitlot_id` int(10) unsigned NOT NULL default '0',
  `site_no` smallint(5) unsigned NOT NULL default '255',
  `sbin_id` int(10) unsigned NOT NULL default '0',
  `nb_parts` int(10) unsigned NOT NULL default '0',
  `nb_runs` int(10) unsigned NOT NULL default '0',
  UNIQUE KEY `uk_wt_stats_sbin_1` (`splitlot_id`,`site_no`,`sbin_id`),
  KEY `fk_wt_stats_sbin_2` (`sbin_id`),
  CONSTRAINT `fk_wt_stats_sbin_1` FOREIGN KEY (`splitlot_id`) REFERENCES `wt_splitlot` (`splitlot_id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_wt_stats_sbin_2` FOREIGN KEY (`sbin_id`) REFERENCES `wt_sbin` (`sbin_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40000 ALTER TABLE `wt_stats_sbin` DISABLE KEYS */;
/*!40000 ALTER TABLE `wt_stats_sbin` ENABLE KEYS */;


DROP TABLE IF EXISTS `wt_stats_summary_ftest`;
CREATE TABLE `wt_stats_summary_ftest` (
  `ftest_info_id` int(10) unsigned NOT NULL default '0',
  `site_no` smallint(5) unsigned NOT NULL default '255',
  `splitlot_id` int(10) unsigned NOT NULL default '0',
  `exec_count` int(10) unsigned default NULL,
  `fail_count` int(10) unsigned default NULL,
  `ttime` int(10) unsigned default NULL,
  UNIQUE KEY `uk_wt_stats_summary_ftest_1` (`ftest_info_id`,`site_no`,`splitlot_id`),
  KEY `fk_wt_stats_summary_ftest_2` (`splitlot_id`),
  CONSTRAINT `fk_wt_stats_summary_ftest_1` FOREIGN KEY (`ftest_info_id`) REFERENCES `wt_ftest_info` (`ftest_info_id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_wt_stats_summary_ftest_2` FOREIGN KEY (`splitlot_id`) REFERENCES `wt_splitlot` (`splitlot_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40000 ALTER TABLE `wt_stats_summary_ftest` DISABLE KEYS */;
/*!40000 ALTER TABLE `wt_stats_summary_ftest` ENABLE KEYS */;


DROP TABLE IF EXISTS `wt_stats_summary_mptest`;
CREATE TABLE `wt_stats_summary_mptest` (
  `mptest_info_id` int(10) unsigned NOT NULL default '0',
  `site_no` smallint(5) unsigned NOT NULL default '0',
  `splitlot_id` int(10) unsigned NOT NULL default '0',
  `exec_count` int(10) unsigned NOT NULL default '0',
  `fail_count` int(10) unsigned NOT NULL default '0',
  `min_value` float default NULL,
  `max_value` float default NULL,
  `sum` float default NULL,
  `square_sum` float default NULL,
  `ttime` int(10) unsigned default NULL,
  UNIQUE KEY `uk_wt_stats_summary_mptest_1` (`mptest_info_id`,`site_no`,`splitlot_id`),
  KEY `fk_wt_stats_summary_mptest_2` (`splitlot_id`),
  CONSTRAINT `fk_wt_stats_summary_mptest_1` FOREIGN KEY (`mptest_info_id`) REFERENCES `wt_mptest_info` (`mptest_info_id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_wt_stats_summary_mptest_2` FOREIGN KEY (`splitlot_id`) REFERENCES `wt_splitlot` (`splitlot_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40000 ALTER TABLE `wt_stats_summary_mptest` DISABLE KEYS */;
/*!40000 ALTER TABLE `wt_stats_summary_mptest` ENABLE KEYS */;


DROP TABLE IF EXISTS `wt_stats_summary_ptest`;
CREATE TABLE `wt_stats_summary_ptest` (
  `ptest_info_id` int(10) unsigned NOT NULL default '0',
  `site_no` smallint(5) unsigned NOT NULL default '0',
  `splitlot_id` int(10) unsigned NOT NULL default '0',
  `exec_count` int(10) unsigned default NULL,
  `fail_count` int(10) unsigned default NULL,
  `min_value` float default NULL,
  `max_value` float default NULL,
  `sum` float default NULL,
  `square_sum` float default NULL,
  `ttime` int(10) unsigned default NULL,
  UNIQUE KEY `uk_wt_stats_summary_ptest_1` (`ptest_info_id`,`site_no`,`splitlot_id`),
  KEY `fk_wt_stats_summary_ptest_2` (`splitlot_id`),
  CONSTRAINT `fk_wt_stats_summary_ptest_1` FOREIGN KEY (`ptest_info_id`) REFERENCES `wt_ptest_info` (`ptest_info_id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_wt_stats_summary_ptest_2` FOREIGN KEY (`splitlot_id`) REFERENCES `wt_splitlot` (`splitlot_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40000 ALTER TABLE `wt_stats_summary_ptest` DISABLE KEYS */;
/*!40000 ALTER TABLE `wt_stats_summary_ptest` ENABLE KEYS */;


DROP TABLE IF EXISTS `wt_wafer_info`;
CREATE TABLE `wt_wafer_info` (
  `splitlot_id` int(10) unsigned NOT NULL default '0',
  `wafer_id` varchar(255) default NULL,
  `fab_id` varchar(255) default NULL,
  `frame_id` varchar(255) default NULL,
  `mask_id` varchar(255) default NULL,
  `wafer_size` float default NULL,
  `die_ht` float default NULL,
  `die_wid` float default NULL,
  `wafer_units` tinyint(3) unsigned default NULL,
  `wafer_flat` char(1) default NULL,
  `center_x` smallint(5) unsigned default NULL,
  `center_y` smallint(5) unsigned default NULL,
  `pos_x` char(1) default NULL,
  `pos_y` char(1) default NULL,
  KEY `fk_wt_wafer_info_1` (`splitlot_id`),
  CONSTRAINT `fk_wt_wafer_info_1` FOREIGN KEY (`splitlot_id`) REFERENCES `wt_splitlot` (`splitlot_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40000 ALTER TABLE `wt_wafer_info` DISABLE KEYS */;
/*!40000 ALTER TABLE `wt_wafer_info` ENABLE KEYS */;


DROP PROCEDURE IF EXISTS `gexdb_purge_procedure`;
DELIMITER $$

CREATE PROCEDURE `gexdb_purge_procedure`(Samples INT, Entry INT)
BEGIN

 	DECLARE DateEraseEntry, DateEraseSamples INT;

	DECLARE SplitlotIdForErase, RunIdForErase INT;

        SELECT Samples INTO DateEraseSamples;

        SELECT Entry INTO DateEraseEntry;

	# Display current value

        SELECT Samples;

        SELECT Entry;



	# @EraseSplitlotSamplesAfterNbDays

	# @EraseSplitlotEntryAfterNbDays

	# Total purge

	# verify if have TotalPurge information else skip this step

	IF NOT (DateEraseEntry IS NULL)

	THEN

		# FOR FT_TABLES

		# Get the Splitlot_Id where to start TotalPurge

		# Get the Run_Id where to start TotalPurge

		SELECT max(splitlot_id) FROM ft_splitlot

			WHERE  UNIX_TIMESTAMP() - UNIX_TIMESTAMP(setup_t) > (DateEraseEntry*24*60*60*100)

			INTO SplitlotIdForErase;



		IF NOT (SplitlotIdForErase IS NULL)

		THEN

			SELECT max(run_id) FROM ft_run

				WHERE  splitlot_id <= SplitlotIdForErase

				INTO RunIdForErase;

			# have the splitlot_id and the run_id for delete

	

			SELECT 'TOTAL PURGE for ft_splitlot';

			SELECT SplitlotIdForErase;

			SELECT RunIdForErase;

			DELETE FROM ft_result_ptest WHERE run_id <= RunIdForErase;

			DELETE FROM ft_result_ftest WHERE run_id <= RunIdForErase;

			DELETE FROM ft_result_mptest WHERE run_id <= RunIdForErase;

			DELETE FROM ft_run WHERE splitlot_id <= SplitlotIdForErase;

			DELETE FROM ft_stats_summary_ptest WHERE splitlot_id <= SplitlotIdForErase;

			DELETE FROM ft_stats_samples_ptest WHERE splitlot_id <= SplitlotIdForErase;

			DELETE FROM ft_ptest_limits WHERE splitlot_id <= SplitlotIdForErase;

			DELETE FROM ft_ptest_info WHERE splitlot_id <= SplitlotIdForErase;

			DELETE FROM ft_stats_summary_ftest WHERE splitlot_id <= SplitlotIdForErase;

			DELETE FROM ft_stats_samples_ftest WHERE splitlot_id <= SplitlotIdForErase;

			DELETE FROM ft_ftest_info WHERE splitlot_id <= SplitlotIdForErase;

			DELETE FROM ft_stats_summary_mptest WHERE splitlot_id <= SplitlotIdForErase;

			DELETE FROM ft_stats_samples_mptest WHERE splitlot_id <= SplitlotIdForErase;

			DELETE FROM ft_mptest_limits WHERE splitlot_id <= SplitlotIdForErase;

			DELETE FROM ft_mptest_info WHERE splitlot_id <= SplitlotIdForErase;

			DELETE FROM ft_stats_sbin WHERE splitlot_id <= SplitlotIdForErase;

			DELETE FROM ft_stats_hbin WHERE splitlot_id <= SplitlotIdForErase;

			DELETE FROM ft_stats_parts WHERE splitlot_id <= SplitlotIdForErase;

			DELETE FROM ft_sbin WHERE splitlot_id <= SplitlotIdForErase;

			DELETE FROM ft_hbin WHERE splitlot_id <= SplitlotIdForErase;

			DELETE FROM ft_splitlot WHERE splitlot_id <= SplitlotIdForErase;

		END IF;



		# FOR WT_TABLES

		# Get the Splitlot_Id where to start TotalPurge

		# Get the Run_Id where to start TotalPurge

		SELECT max(splitlot_id) FROM wt_splitlot

			WHERE  UNIX_TIMESTAMP() - UNIX_TIMESTAMP(setup_t) > (DateEraseEntry*24*60*60*100)

			INTO SplitlotIdForErase;

		IF NOT (SplitlotIdForErase IS NULL)

		THEN

			SELECT max(run_id) FROM wt_run

				WHERE  splitlot_id <= SplitlotIdForErase

				INTO RunIdForErase;

			# have the splitlot_id and the run_id for delete

	

	                SELECT 'TOTAL PURGE for wt_splitlot';

	                SELECT SplitlotIdForErase;

	                SELECT RunIdForErase;

			DELETE FROM wt_result_ptest WHERE run_id <= RunIdForErase;

			DELETE FROM wt_result_ftest WHERE run_id <= RunIdForErase;

			DELETE FROM wt_result_mptest WHERE run_id <= RunIdForErase;

			DELETE FROM wt_run WHERE splitlot_id <= SplitlotIdForErase;

			DELETE FROM wt_stats_summary_ptest WHERE splitlot_id <= SplitlotIdForErase;

			DELETE FROM wt_stats_samples_ptest WHERE splitlot_id <= SplitlotIdForErase;

			DELETE FROM wt_ptest_limits WHERE splitlot_id <= SplitlotIdForErase;

			DELETE FROM wt_ptest_info WHERE splitlot_id <= SplitlotIdForErase;

			DELETE FROM wt_stats_summary_ftest WHERE splitlot_id <= SplitlotIdForErase;

			DELETE FROM wt_stats_samples_ftest WHERE splitlot_id <= SplitlotIdForErase;

			DELETE FROM wt_ftest_info WHERE splitlot_id <= SplitlotIdForErase;

			DELETE FROM wt_stats_summary_mptest WHERE splitlot_id <= SplitlotIdForErase;

			DELETE FROM wt_stats_samples_mptest WHERE splitlot_id <= SplitlotIdForErase;

			DELETE FROM wt_mptest_limits WHERE splitlot_id <= SplitlotIdForErase;

			DELETE FROM wt_mptest_info WHERE splitlot_id <= SplitlotIdForErase;

			DELETE FROM wt_stats_sbin WHERE splitlot_id <= SplitlotIdForErase;

			DELETE FROM wt_stats_hbin WHERE splitlot_id <= SplitlotIdForErase;

			DELETE FROM wt_stats_parts WHERE splitlot_id <= SplitlotIdForErase;

			DELETE FROM wt_sbin WHERE splitlot_id <= SplitlotIdForErase;

			DELETE FROM wt_hbin WHERE splitlot_id <= SplitlotIdForErase;

			DELETE FROM wt_wafer_info WHERE splitlot_id <= SplitlotIdForErase;

			DELETE FROM wt_splitlot WHERE splitlot_id <= SplitlotIdForErase;

		END IF;



		# FOR ET_TABLES

		# Get the Splitlot_Id where to start TotalPurge

		# Get the Run_Id where to start TotalPurge

		SELECT max(splitlot_id) FROM et_splitlot

			WHERE  UNIX_TIMESTAMP() - UNIX_TIMESTAMP(setup_t) > (DateEraseEntry*24*60*60*100)

			INTO SplitlotIdForErase;



		IF NOT (SplitlotIdForErase IS NULL)

		THEN

			SELECT max(run_id) FROM et_run

				WHERE  splitlot_id <= SplitlotIdForErase

				INTO RunIdForErase;

			# have the splitlot_id and the run_id for delete

	

	                SELECT 'TOTAL PURGE for et_splitlot';

	                SELECT SplitlotIdForErase;

	                SELECT RunIdForErase;

			DELETE FROM et_result_ptest WHERE run_id <= RunIdForErase;

			DELETE FROM et_run WHERE splitlot_id <= SplitlotIdForErase;

			DELETE FROM et_stats_summary_ptest WHERE splitlot_id <= SplitlotIdForErase;

			DELETE FROM et_stats_samples_ptest WHERE splitlot_id <= SplitlotIdForErase;

			DELETE FROM et_ptest_limits WHERE splitlot_id <= SplitlotIdForErase;

			DELETE FROM et_ptest_info WHERE splitlot_id <= SplitlotIdForErase;

			DELETE FROM et_stats_sbin WHERE splitlot_id <= SplitlotIdForErase;

			DELETE FROM et_stats_hbin WHERE splitlot_id <= SplitlotIdForErase;

			DELETE FROM et_stats_parts WHERE splitlot_id <= SplitlotIdForErase;

			DELETE FROM et_sbin WHERE splitlot_id <= SplitlotIdForErase;

			DELETE FROM et_hbin WHERE splitlot_id <= SplitlotIdForErase;

			DELETE FROM et_wafer_info WHERE splitlot_id <= SplitlotIdForErase;

			DELETE FROM et_splitlot WHERE splitlot_id <= SplitlotIdForErase;

		END IF;

	END IF;

	# Samples purge

	# verify if have Samplesge information else skip this step

	IF NOT (DateEraseSamples IS NULL)

	THEN

		# FOR FT_TABLES

		# Get the Splitlot_Id where to start PartialPurge

		# Get the Run_Id where to start PartialPurge

		SELECT max(splitlot_id) FROM ft_splitlot

			WHERE  UNIX_TIMESTAMP() - UNIX_TIMESTAMP(setup_t) > (DateEraseSamples*24*60*60*100)

			INTO SplitlotIdForErase;



		IF NOT (SplitlotIdForErase IS NULL)

		THEN

			SELECT max(run_id) FROM ft_run

				WHERE  splitlot_id <= SplitlotIdForErase

				INTO RunIdForErase;

			# have the splitlot_id and the run_id for delete

	

	                SELECT 'PARTIAL PURGE for ft_splitlot';

	                SELECT SplitlotIdForErase;

	                SELECT RunIdForErase;

			DELETE FROM ft_result_ptest WHERE run_id <= RunIdForErase;

			DELETE FROM ft_result_ftest WHERE run_id <= RunIdForErase;

			DELETE FROM ft_result_mptest WHERE run_id <= RunIdForErase;

			DELETE FROM ft_run WHERE splitlot_id <= SplitlotIdForErase;

		END IF;



		# FOR WT_TABLES

		# Get the Splitlot_Id where to start PartialPurge

		# Get the Run_Id where to start PartialPurge

		SELECT max(splitlot_id) FROM wt_splitlot

			WHERE  UNIX_TIMESTAMP() - UNIX_TIMESTAMP(setup_t) > (DateEraseSamples*24*60*60*100)

			INTO SplitlotIdForErase;



		IF NOT (SplitlotIdForErase IS NULL)

		THEN

			SELECT max(run_id) FROM wt_run

				WHERE  splitlot_id <= SplitlotIdForErase

				INTO RunIdForErase;

			# have the splitlot_id and the run_id for delete

		

	                SELECT 'PARTIAL PURGE for wt_splitlot';

	                SELECT SplitlotIdForErase;

	                SELECT RunIdForErase;

			DELETE FROM wt_result_ptest WHERE run_id <= RunIdForErase;

			DELETE FROM wt_result_ftest WHERE run_id <= RunIdForErase;

			DELETE FROM wt_result_mptest WHERE run_id <= RunIdForErase;

			DELETE FROM wt_run WHERE splitlot_id <= SplitlotIdForErase;

		END IF;

	

		# FOR E_TABLES

		# Get the Splitlot_Id where to start PartialPurge

		# Get the Run_Id where to start PartialPurge

		SELECT max(splitlot_id) FROM et_splitlot

			WHERE  UNIX_TIMESTAMP() - UNIX_TIMESTAMP(setup_t) > (DateEraseSamples*24*60*60*100)

			INTO SplitlotIdForErase;



		IF NOT (SplitlotIdForErase IS NULL)

		THEN

			SELECT max(run_id) FROM et_run

				WHERE  splitlot_id <= SplitlotIdForErase

				INTO RunIdForErase;

			# have the splitlot_id and the run_id for delete

	

	                SELECT 'PARTIAL PURGE for et_splitlot';

	                SELECT SplitlotIdForErase;

	                SELECT RunIdForErase;

			DELETE FROM et_result_ptest WHERE run_id <= RunIdForErase;

			DELETE FROM et_run WHERE splitlot_id <= SplitlotIdForErase;

		END IF;	

	END IF;



END $$
DROP PROCEDURE IF EXISTS `gexdb_select_test_procedure`$$

CREATE PROCEDURE `gexdb_select_test_procedure`()
BEGIN
	# Get the Splitlot_Id where to start SamplesPurge
	DECLARE SplitlotIdForSelect INT;
	# Get the Run_Id where to start SamplesPurge
	DECLARE MinRunIdForSelect INT;
	DECLARE MaxRunIdForSelect INT;
	# for ft_tables
	SELECT max(splitlot_id)
                FROM ft_splitlot INTO SplitlotIdForSelect;
	BEGIN
		DECLARE a INT;
		DECLARE b INT;
		DECLARE c BINARY;
		DECLARE d FLOAT;
		DECLARE MySum INT;
		SELECT max(run_id)
                        FROM ft_run
                        WHERE  splitlot_id = SplitlotIdForSelect INTO MaxRunIdForSelect;
		SELECT min(run_id)
                        FROM ft_run
                        WHERE  splitlot_id = SplitlotIdForSelect INTO MinRunIdForSelect;

		# have the splitlot_id and the run_id for selection

                SELECT 'SELECTION';
                SELECT SplitlotIdForSelect;
                SELECT MinRunIdForSelect;
                SELECT MaxRunIdForSelect;

		BEGIN

			DECLARE not_found BOOLEAN DEFAULT false;

			DECLARE cur10 CURSOR FOR SELECT run_id FROM ft_result_ptest WHERE run_id <= MaxRunIdForSelect AND run_id >= MinRunIdForSelect;

			DECLARE CONTINUE HANDLER FOR NOT FOUND SET not_found = TRUE;

			OPEN cur10;
			SET not_found = false;
			REPEAT
				FETCH cur10 INTO a;
				SET MySum = a; 
			UNTIL not_found = true	END REPEAT;
			CLOSE cur10;

		END;
	END;

END $$
DELIMITER ;
/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
