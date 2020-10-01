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
  `HBIN_ID` int(10) unsigned NOT NULL auto_increment,
  `SPLITLOT_ID` int(10) unsigned NOT NULL default '0',
  `HBIN_NO` smallint(5) unsigned NOT NULL default '0',
  `HBIN_NAME` varchar(255) NOT NULL default '',
  `HBIN_CAT` char(1) default NULL,
  `NB_PARTS` int(10) unsigned NOT NULL default '0',
  `NB_RUNS` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`HBIN_ID`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE `et_hbin` DISABLE KEYS */;
/*!40000 ALTER TABLE `et_hbin` ENABLE KEYS */;


DROP TABLE IF EXISTS `et_lot`;
CREATE TABLE `et_lot` (
  `LOT_ID` varchar(255) NOT NULL default '',
  `PRODUCT_NAME` varchar(255) default NULL,
  `NB_PARTS` int(10) unsigned NOT NULL default '0',
  `NB_PARTS_GOOD` int(10) unsigned NOT NULL default '0',
  `NB_RUNS` int(10) unsigned NOT NULL default '0',
  `NB_RUNS_GOOD` int(10) unsigned NOT NULL default '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE `et_lot` DISABLE KEYS */;
/*!40000 ALTER TABLE `et_lot` ENABLE KEYS */;


DROP TABLE IF EXISTS `et_ptest_info`;
CREATE TABLE `et_ptest_info` (
  `PTEST_INFO_ID` int(10) unsigned NOT NULL auto_increment,
  `SPLITLOT_ID` int(10) unsigned NOT NULL default '0',
  `TNUM` int(10) unsigned NOT NULL default '0',
  `TNAME` varchar(255) NOT NULL default '',
  `UNITS` varchar(255) NOT NULL default '',
  `FLAGS` binary(1) NOT NULL default '\0',
  `LL` float default NULL,
  `HL` float default NULL,
  `TESTSEQ` smallint(5) unsigned default NULL,
  PRIMARY KEY  (`PTEST_INFO_ID`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE `et_ptest_info` DISABLE KEYS */;
/*!40000 ALTER TABLE `et_ptest_info` ENABLE KEYS */;


DROP TABLE IF EXISTS `et_ptest_limits`;
CREATE TABLE `et_ptest_limits` (
  `PTEST_INFO_ID` int(10) unsigned NOT NULL default '0',
  `SITE_NO` smallint(5) unsigned NOT NULL default '255',
  `SPLITLOT_ID` int(10) unsigned NOT NULL default '0',
  `LL` float NOT NULL default '0',
  `HL` float NOT NULL default '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE `et_ptest_limits` DISABLE KEYS */;
/*!40000 ALTER TABLE `et_ptest_limits` ENABLE KEYS */;


DROP TABLE IF EXISTS `et_result_ptest`;
CREATE TABLE `et_result_ptest` (
  `PTEST_INFO_ID` int(10) unsigned NOT NULL default '0',
  `RUN_ID` int(10) unsigned NOT NULL default '255',
  `FLAGS` binary(1) NOT NULL default '\0',
  `VALUE` float default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE `et_result_ptest` DISABLE KEYS */;
/*!40000 ALTER TABLE `et_result_ptest` ENABLE KEYS */;


DROP TABLE IF EXISTS `et_run`;
CREATE TABLE `et_run` (
  `RUN_ID` int(10) unsigned NOT NULL auto_increment,
  `SPLITLOT_ID` int(10) unsigned NOT NULL default '0',
  `SITE_NO` smallint(5) unsigned NOT NULL default '255',
  `PART_ID` varchar(255) default NULL,
  `PART_X` smallint(6) default NULL,
  `PART_Y` smallint(6) default NULL,
  `HBIN_NO` smallint(5) unsigned NOT NULL default '0',
  `SBIN_NO` smallint(5) unsigned default NULL,
  `TTIME` int(10) unsigned default NULL,
  `TESTS_EXECUTED` smallint(5) unsigned NOT NULL default '0',
  `TESTS_FAILED` smallint(5) unsigned NOT NULL default '0',
  `FIRSTFAIL_TNUM` int(10) unsigned default NULL,
  `FIRSTFAIL_TNAME` varchar(255) default NULL,
  `RETEST_INDEX` tinyint(3) unsigned NOT NULL default '0',
  PRIMARY KEY  (`RUN_ID`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE `et_run` DISABLE KEYS */;
/*!40000 ALTER TABLE `et_run` ENABLE KEYS */;


DROP TABLE IF EXISTS `et_sbin`;
CREATE TABLE `et_sbin` (
  `SBIN_ID` int(10) unsigned NOT NULL auto_increment,
  `SPLITLOT_ID` int(10) unsigned NOT NULL default '0',
  `SBIN_NO` smallint(5) unsigned NOT NULL default '0',
  `SBIN_NAME` varchar(255) NOT NULL default '',
  `SBIN_CAT` char(1) default NULL,
  `NB_PARTS` int(10) unsigned NOT NULL default '0',
  `NB_RUNS` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`SBIN_ID`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE `et_sbin` DISABLE KEYS */;
/*!40000 ALTER TABLE `et_sbin` ENABLE KEYS */;


DROP TABLE IF EXISTS `et_splitlot`;
CREATE TABLE `et_splitlot` (
  `SPLITLOT_ID` int(10) unsigned NOT NULL auto_increment,
  `LOT_ID` varchar(255) NOT NULL default '0',
  `SUBLOT_ID` varchar(255) NOT NULL default '',
  `WAFER_ID` varchar(255) default NULL,
  `SETUP_T` int(10) unsigned NOT NULL default '0',
  `START_T` int(10) unsigned NOT NULL default '0',
  `FINISH_T` int(10) unsigned NOT NULL default '0',
  `STAT_NUM` tinyint(3) unsigned NOT NULL default '0',
  `TESTER_NAME` varchar(255) NOT NULL default '',
  `FLAGS` binary(2) NOT NULL default '\0\0',
  `NB_PARTS` int(10) unsigned NOT NULL default '0',
  `NB_PARTS_GOOD` int(10) unsigned NOT NULL default '0',
  `NB_RUNS` int(10) unsigned NOT NULL default '0',
  `NB_RUNS_GOOD` int(10) unsigned NOT NULL default '0',
  `DATA_ORIGIN` tinyint(3) unsigned NOT NULL default '0',
  `RETEST_INDEX` tinyint(3) unsigned NOT NULL default '0',
  `RETEST_HBINS` varchar(255) default NULL,
  `REWORK_CODE` tinyint(3) unsigned NOT NULL default '0',
  `JOB_NAM` varchar(255) NOT NULL default '',
  `JOB_REV` varchar(255) NOT NULL default '',
  `OPER_NAM` varchar(255) NOT NULL default '',
  `EXEC_TYP` varchar(255) NOT NULL default '',
  `EXEC_VER` varchar(255) NOT NULL default '',
  `TEST_COD` varchar(255) NOT NULL default '',
  `FACIL_ID` varchar(255) NOT NULL default '',
  `TST_TEMP` varchar(255) NOT NULL default '',
  `NB_SITES` tinyint(3) unsigned NOT NULL default '1',
  `HEAD_NUM` tinyint(3) unsigned default NULL,
  `HANDLER_ID` varchar(255) default NULL,
  `LOADBOARD_ID` varchar(255) default NULL,
  `DIB_ID` varchar(255) default NULL,
  `CARD_ID` varchar(255) default NULL,
  `FILE_HOST_ID` int(10) unsigned default '0',
  `FILE_PATH` varchar(255) NOT NULL default '',
  `FILE_NAME` varchar(255) NOT NULL default '',
  PRIMARY KEY  (`SPLITLOT_ID`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE `et_splitlot` DISABLE KEYS */;
/*!40000 ALTER TABLE `et_splitlot` ENABLE KEYS */;


DROP TABLE IF EXISTS `et_stats_hbin`;
CREATE TABLE `et_stats_hbin` (
  `SPLITLOT_ID` int(10) unsigned NOT NULL default '0',
  `SITE_NO` smallint(5) unsigned NOT NULL default '255',
  `HBIN_ID` int(10) unsigned NOT NULL default '0',
  `NB_PARTS` int(10) unsigned NOT NULL default '0',
  `NB_RUNS` int(10) unsigned NOT NULL default '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE `et_stats_hbin` DISABLE KEYS */;
/*!40000 ALTER TABLE `et_stats_hbin` ENABLE KEYS */;


DROP TABLE IF EXISTS `et_stats_parts`;
CREATE TABLE `et_stats_parts` (
  `SPLITLOT_ID` int(10) unsigned NOT NULL default '0',
  `SITE_NO` smallint(5) unsigned NOT NULL default '255',
  `NB_RUNS` int(10) unsigned NOT NULL default '0',
  `NB_RUNS_GOOD` int(10) unsigned NOT NULL default '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE `et_stats_parts` DISABLE KEYS */;
/*!40000 ALTER TABLE `et_stats_parts` ENABLE KEYS */;


DROP TABLE IF EXISTS `et_stats_samples_ptest`;
CREATE TABLE `et_stats_samples_ptest` (
  `PTEST_INFO_ID` int(10) unsigned NOT NULL default '0',
  `SITE_NO` smallint(5) unsigned NOT NULL default '0',
  `SPLITLOT_ID` int(10) unsigned NOT NULL default '0',
  `EXEC_COUNT` int(10) unsigned NOT NULL default '0',
  `FAIL_COUNT` int(10) unsigned default NULL,
  `MIN_VALUE` float default NULL,
  `MAX_VALUE` float default NULL,
  `SUM` float default NULL,
  `SQUARE_SUM` float default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE `et_stats_samples_ptest` DISABLE KEYS */;
/*!40000 ALTER TABLE `et_stats_samples_ptest` ENABLE KEYS */;


DROP TABLE IF EXISTS `et_stats_sbin`;
CREATE TABLE `et_stats_sbin` (
  `SPLITLOT_ID` int(10) unsigned NOT NULL default '0',
  `SITE_NO` smallint(5) unsigned NOT NULL default '255',
  `SBIN_ID` int(10) unsigned NOT NULL default '0',
  `NB_PARTS` int(10) unsigned NOT NULL default '0',
  `NB_RUNS` int(10) unsigned NOT NULL default '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE `et_stats_sbin` DISABLE KEYS */;
/*!40000 ALTER TABLE `et_stats_sbin` ENABLE KEYS */;


DROP TABLE IF EXISTS `et_stats_summary_ptest`;
CREATE TABLE `et_stats_summary_ptest` (
  `PTEST_INFO_ID` int(10) unsigned NOT NULL default '0',
  `SITE_NO` smallint(5) unsigned NOT NULL default '0',
  `SPLITLOT_ID` int(10) unsigned NOT NULL default '0',
  `EXEC_COUNT` int(10) unsigned default NULL,
  `FAIL_COUNT` int(10) unsigned default NULL,
  `MIN_VALUE` float default NULL,
  `MAX_VALUE` float default NULL,
  `SUM` float default NULL,
  `SQUARE_SUM` float default NULL,
  `TTIME` int(10) unsigned default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE `et_stats_summary_ptest` DISABLE KEYS */;
/*!40000 ALTER TABLE `et_stats_summary_ptest` ENABLE KEYS */;


DROP TABLE IF EXISTS `et_wafer_info`;
CREATE TABLE `et_wafer_info` (
  `SPLITLOT_ID` int(10) unsigned NOT NULL default '0',
  `WAFER_ID` varchar(255) default NULL,
  `FAB_ID` varchar(255) default NULL,
  `FRAME_ID` varchar(255) default NULL,
  `MASK_ID` varchar(255) default NULL,
  `WAFER_SIZE` float default NULL,
  `DIE_HT` float default NULL,
  `DIE_WID` float default NULL,
  `WAFER_UNITS` tinyint(3) unsigned default NULL,
  `WAFER_FLAT` char(1) default NULL,
  `CENTER_X` smallint(5) unsigned default NULL,
  `CENTER_Y` smallint(5) unsigned default NULL,
  `POS_X` char(1) default NULL,
  `POS_Y` char(1) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE `et_wafer_info` DISABLE KEYS */;
/*!40000 ALTER TABLE `et_wafer_info` ENABLE KEYS */;


DROP TABLE IF EXISTS `file_host`;
CREATE TABLE `file_host` (
  `FILE_HOST_ID` int(10) unsigned NOT NULL auto_increment,
  `HOST_NAME` varchar(255) NOT NULL default '',
  `HOST_FTPUSER` varchar(255) NOT NULL default '',
  `HOST_FTPPASSWORD` varchar(255) NOT NULL default '',
  `HOST_FTPPATH` varchar(255) default NULL,
  PRIMARY KEY  (`FILE_HOST_ID`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE `file_host` DISABLE KEYS */;
/*!40000 ALTER TABLE `file_host` ENABLE KEYS */;


DROP TABLE IF EXISTS `ft_ftest_info`;
CREATE TABLE `ft_ftest_info` (
  `FTEST_INFO_ID` int(10) unsigned NOT NULL auto_increment,
  `SPLITLOT_ID` int(10) unsigned NOT NULL default '0',
  `TNUM` int(10) unsigned NOT NULL default '0',
  `TNAME` varchar(255) NOT NULL default '',
  `TESTSEQ` smallint(5) unsigned default NULL,
  PRIMARY KEY  (`FTEST_INFO_ID`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE `ft_ftest_info` DISABLE KEYS */;
/*!40000 ALTER TABLE `ft_ftest_info` ENABLE KEYS */;


DROP TABLE IF EXISTS `ft_hbin`;
CREATE TABLE `ft_hbin` (
  `HBIN_ID` int(10) unsigned NOT NULL auto_increment,
  `SPLITLOT_ID` int(10) unsigned NOT NULL default '0',
  `HBIN_NO` smallint(5) unsigned NOT NULL default '0',
  `HBIN_NAME` varchar(255) NOT NULL default '',
  `HBIN_CAT` char(1) default NULL,
  `NB_PARTS` int(10) unsigned NOT NULL default '0',
  `NB_RUNS` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`HBIN_ID`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE `ft_hbin` DISABLE KEYS */;
/*!40000 ALTER TABLE `ft_hbin` ENABLE KEYS */;


DROP TABLE IF EXISTS `ft_lot`;
CREATE TABLE `ft_lot` (
  `LOT_ID` varchar(255) NOT NULL default '',
  `PRODUCT_NAME` varchar(255) default NULL,
  `NB_PARTS` int(10) unsigned NOT NULL default '0',
  `NB_PARTS_GOOD` int(10) unsigned NOT NULL default '0',
  `NB_RUNS` int(10) unsigned NOT NULL default '0',
  `NB_RUNS_GOOD` int(10) unsigned NOT NULL default '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE `ft_lot` DISABLE KEYS */;
/*!40000 ALTER TABLE `ft_lot` ENABLE KEYS */;


DROP TABLE IF EXISTS `ft_mptest_info`;
CREATE TABLE `ft_mptest_info` (
  `MPTEST_INFO_ID` int(10) unsigned NOT NULL auto_increment,
  `SPLITLOT_ID` int(10) unsigned NOT NULL default '0',
  `TNUM` int(10) unsigned NOT NULL default '0',
  `TNAME` varchar(255) NOT NULL default '',
  `TPIN` smallint(6) NOT NULL default '0',
  `UNITS` varchar(255) NOT NULL default '',
  `FLAGS` binary(1) NOT NULL default '\0',
  `LL` float default NULL,
  `HL` float default NULL,
  `TESTSEQ` smallint(5) unsigned default NULL,
  PRIMARY KEY  (`MPTEST_INFO_ID`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE `ft_mptest_info` DISABLE KEYS */;
/*!40000 ALTER TABLE `ft_mptest_info` ENABLE KEYS */;


DROP TABLE IF EXISTS `ft_mptest_limits`;
CREATE TABLE `ft_mptest_limits` (
  `MPTEST_INFO_ID` int(10) unsigned NOT NULL default '0',
  `SITE_NO` smallint(5) unsigned NOT NULL default '255',
  `SPLITLOT_ID` int(10) unsigned NOT NULL default '0',
  `LL` float NOT NULL default '0',
  `HL` float NOT NULL default '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE `ft_mptest_limits` DISABLE KEYS */;
/*!40000 ALTER TABLE `ft_mptest_limits` ENABLE KEYS */;


DROP TABLE IF EXISTS `ft_ptest_info`;
CREATE TABLE `ft_ptest_info` (
  `PTEST_INFO_ID` int(10) unsigned NOT NULL auto_increment,
  `SPLITLOT_ID` int(10) unsigned NOT NULL default '0',
  `TNUM` int(10) unsigned NOT NULL default '0',
  `TNAME` varchar(255) NOT NULL default '',
  `UNITS` varchar(255) NOT NULL default '',
  `FLAGS` binary(1) NOT NULL default '\0',
  `LL` float default NULL,
  `HL` float default NULL,
  `TESTSEQ` smallint(5) unsigned default NULL,
  PRIMARY KEY  (`PTEST_INFO_ID`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE `ft_ptest_info` DISABLE KEYS */;
/*!40000 ALTER TABLE `ft_ptest_info` ENABLE KEYS */;


DROP TABLE IF EXISTS `ft_ptest_limits`;
CREATE TABLE `ft_ptest_limits` (
  `PTEST_INFO_ID` int(10) unsigned NOT NULL default '0',
  `SITE_NO` smallint(5) unsigned NOT NULL default '255',
  `SPLITLOT_ID` int(10) unsigned NOT NULL default '0',
  `LL` float NOT NULL default '0',
  `HL` float NOT NULL default '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE `ft_ptest_limits` DISABLE KEYS */;
/*!40000 ALTER TABLE `ft_ptest_limits` ENABLE KEYS */;


DROP TABLE IF EXISTS `ft_result_ftest`;
CREATE TABLE `ft_result_ftest` (
  `FTEST_INFO_ID` int(10) unsigned NOT NULL,
  `RUN_ID` int(10) unsigned NOT NULL default '255',
  `FLAGS` binary(1) NOT NULL default '\0',
  `VECT_NAM` varchar(255) NOT NULL default '',
  `VECT_OFF` smallint(6) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE `ft_result_ftest` DISABLE KEYS */;
/*!40000 ALTER TABLE `ft_result_ftest` ENABLE KEYS */;


DROP TABLE IF EXISTS `ft_result_mptest`;
CREATE TABLE `ft_result_mptest` (
  `MPTEST_INFO_ID` int(10) unsigned NOT NULL default '0',
  `RUN_ID` int(10) unsigned NOT NULL default '255',
  `FLAGS` binary(1) NOT NULL default '\0',
  `VALUE` float NOT NULL default '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE `ft_result_mptest` DISABLE KEYS */;
/*!40000 ALTER TABLE `ft_result_mptest` ENABLE KEYS */;


DROP TABLE IF EXISTS `ft_result_ptest`;
CREATE TABLE `ft_result_ptest` (
  `PTEST_INFO_ID` int(10) unsigned NOT NULL default '0',
  `RUN_ID` int(10) unsigned NOT NULL default '255',
  `FLAGS` binary(1) NOT NULL default '\0',
  `VALUE` float default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE `ft_result_ptest` DISABLE KEYS */;
/*!40000 ALTER TABLE `ft_result_ptest` ENABLE KEYS */;


DROP TABLE IF EXISTS `ft_run`;
CREATE TABLE `ft_run` (
  `RUN_ID` int(10) unsigned NOT NULL auto_increment,
  `SPLITLOT_ID` int(10) unsigned NOT NULL default '0',
  `SITE_NO` smallint(5) unsigned NOT NULL default '255',
  `PART_ID` varchar(255) default NULL,
  `PART_X` smallint(6) default NULL,
  `PART_Y` smallint(6) default NULL,
  `HBIN_NO` smallint(5) unsigned NOT NULL default '0',
  `SBIN_NO` smallint(5) unsigned default NULL,
  `TTIME` int(10) unsigned default NULL,
  `TESTS_EXECUTED` smallint(5) unsigned NOT NULL default '0',
  `TESTS_FAILED` smallint(5) unsigned NOT NULL default '0',
  `FIRSTFAIL_TNUM` int(10) unsigned default NULL,
  `FIRSTFAIL_TNAME` varchar(255) default NULL,
  `RETEST_INDEX` tinyint(3) unsigned NOT NULL default '0',
  PRIMARY KEY  (`RUN_ID`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE `ft_run` DISABLE KEYS */;
/*!40000 ALTER TABLE `ft_run` ENABLE KEYS */;


DROP TABLE IF EXISTS `ft_sbin`;
CREATE TABLE `ft_sbin` (
  `SBIN_ID` int(10) unsigned NOT NULL auto_increment,
  `SPLITLOT_ID` int(10) unsigned NOT NULL default '0',
  `SBIN_NO` smallint(5) unsigned NOT NULL default '0',
  `SBIN_NAME` varchar(255) NOT NULL default '',
  `SBIN_CAT` char(1) default NULL,
  `NB_PARTS` int(10) unsigned NOT NULL default '0',
  `NB_RUNS` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`SBIN_ID`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE `ft_sbin` DISABLE KEYS */;
/*!40000 ALTER TABLE `ft_sbin` ENABLE KEYS */;


DROP TABLE IF EXISTS `ft_splitlot`;
CREATE TABLE `ft_splitlot` (
  `SPLITLOT_ID` int(10) unsigned NOT NULL auto_increment,
  `LOT_ID` varchar(255) NOT NULL default '0',
  `SUBLOT_ID` varchar(255) NOT NULL default '',
  `SETUP_T` int(10) unsigned NOT NULL default '0',
  `START_T` int(10) unsigned NOT NULL default '0',
  `FINISH_T` int(10) unsigned NOT NULL default '0',
  `STAT_NUM` tinyint(3) unsigned NOT NULL default '0',
  `TESTER_NAME` varchar(255) NOT NULL default '',
  `FLAGS` binary(2) NOT NULL default '\0\0',
  `NB_PARTS` int(10) unsigned NOT NULL default '0',
  `NB_PARTS_GOOD` int(10) unsigned NOT NULL default '0',
  `NB_RUNS` int(10) unsigned NOT NULL default '0',
  `NB_RUNS_GOOD` int(10) unsigned NOT NULL default '0',
  `DATA_ORIGIN` tinyint(3) unsigned NOT NULL default '0',
  `RETEST_INDEX` tinyint(3) unsigned NOT NULL default '0',
  `RETEST_HBINS` varchar(255) default NULL,
  `REWORK_CODE` tinyint(3) unsigned NOT NULL default '0',
  `JOB_NAM` varchar(255) NOT NULL default '',
  `JOB_REV` varchar(255) NOT NULL default '',
  `OPER_NAM` varchar(255) NOT NULL default '',
  `EXEC_TYP` varchar(255) NOT NULL default '',
  `EXEC_VER` varchar(255) NOT NULL default '',
  `TEST_COD` varchar(255) NOT NULL default '',
  `FACIL_ID` varchar(255) NOT NULL default '',
  `TST_TEMP` varchar(255) NOT NULL default '',
  `NB_SITES` tinyint(3) unsigned NOT NULL default '1',
  `HEAD_NUM` tinyint(3) unsigned default NULL,
  `HANDLER_ID` varchar(255) default NULL,
  `LOADBOARD_ID` varchar(255) default NULL,
  `DIB_ID` varchar(255) default NULL,
  `CARD_ID` varchar(255) default NULL,
  `FILE_HOST_ID` int(10) unsigned default '0',
  `FILE_PATH` varchar(255) NOT NULL default '',
  `FILE_NAME` varchar(255) NOT NULL default '',
  PRIMARY KEY  (`SPLITLOT_ID`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE `ft_splitlot` DISABLE KEYS */;
/*!40000 ALTER TABLE `ft_splitlot` ENABLE KEYS */;


DROP TABLE IF EXISTS `ft_stats_hbin`;
CREATE TABLE `ft_stats_hbin` (
  `SPLITLOT_ID` int(10) unsigned NOT NULL default '0',
  `SITE_NO` smallint(5) unsigned NOT NULL default '255',
  `HBIN_ID` int(10) unsigned NOT NULL default '0',
  `NB_PARTS` int(10) unsigned NOT NULL default '0',
  `NB_RUNS` int(10) unsigned NOT NULL default '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE `ft_stats_hbin` DISABLE KEYS */;
/*!40000 ALTER TABLE `ft_stats_hbin` ENABLE KEYS */;


DROP TABLE IF EXISTS `ft_stats_parts`;
CREATE TABLE `ft_stats_parts` (
  `SPLITLOT_ID` int(10) unsigned NOT NULL default '0',
  `SITE_NO` smallint(5) unsigned NOT NULL default '255',
  `NB_RUNS` int(10) unsigned NOT NULL default '0',
  `NB_RUNS_GOOD` int(10) unsigned NOT NULL default '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE `ft_stats_parts` DISABLE KEYS */;
/*!40000 ALTER TABLE `ft_stats_parts` ENABLE KEYS */;


DROP TABLE IF EXISTS `ft_stats_samples_ftest`;
CREATE TABLE `ft_stats_samples_ftest` (
  `FTEST_INFO_ID` int(10) unsigned NOT NULL default '0',
  `SITE_NO` smallint(5) unsigned NOT NULL default '255',
  `SPLITLOT_ID` int(10) unsigned NOT NULL default '0',
  `EXEC_COUNT` int(10) unsigned NOT NULL default '0',
  `FAIL_COUNT` int(10) unsigned default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE `ft_stats_samples_ftest` DISABLE KEYS */;
/*!40000 ALTER TABLE `ft_stats_samples_ftest` ENABLE KEYS */;


DROP TABLE IF EXISTS `ft_stats_samples_mptest`;
CREATE TABLE `ft_stats_samples_mptest` (
  `MPTEST_INFO_ID` int(10) unsigned NOT NULL default '0',
  `SITE_NO` smallint(5) unsigned NOT NULL default '255',
  `SPLITLOT_ID` int(10) unsigned NOT NULL default '0',
  `EXEC_COUNT` int(10) unsigned NOT NULL default '0',
  `FAIL_COUNT` int(10) unsigned default NULL,
  `MIN_VALUE` float default NULL,
  `MAX_VALUE` float default NULL,
  `SUM` float default NULL,
  `SQUARE_SUM` float default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE `ft_stats_samples_mptest` DISABLE KEYS */;
/*!40000 ALTER TABLE `ft_stats_samples_mptest` ENABLE KEYS */;


DROP TABLE IF EXISTS `ft_stats_samples_ptest`;
CREATE TABLE `ft_stats_samples_ptest` (
  `PTEST_INFO_ID` int(10) unsigned NOT NULL default '0',
  `SITE_NO` smallint(5) unsigned NOT NULL default '0',
  `SPLITLOT_ID` int(10) unsigned NOT NULL default '0',
  `EXEC_COUNT` int(10) unsigned NOT NULL default '0',
  `FAIL_COUNT` int(10) unsigned default NULL,
  `MIN_VALUE` float default NULL,
  `MAX_VALUE` float default NULL,
  `SUM` float default NULL,
  `SQUARE_SUM` float default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE `ft_stats_samples_ptest` DISABLE KEYS */;
/*!40000 ALTER TABLE `ft_stats_samples_ptest` ENABLE KEYS */;


DROP TABLE IF EXISTS `ft_stats_sbin`;
CREATE TABLE `ft_stats_sbin` (
  `SPLITLOT_ID` int(10) unsigned NOT NULL default '0',
  `SITE_NO` smallint(5) unsigned NOT NULL default '255',
  `SBIN_ID` int(10) unsigned NOT NULL default '0',
  `NB_PARTS` int(10) unsigned NOT NULL default '0',
  `NB_RUNS` int(10) unsigned NOT NULL default '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE `ft_stats_sbin` DISABLE KEYS */;
/*!40000 ALTER TABLE `ft_stats_sbin` ENABLE KEYS */;


DROP TABLE IF EXISTS `ft_stats_summary_ftest`;
CREATE TABLE `ft_stats_summary_ftest` (
  `FTEST_INFO_ID` int(10) unsigned NOT NULL default '0',
  `SITE_NO` smallint(5) unsigned NOT NULL default '255',
  `SPLITLOT_ID` int(10) unsigned NOT NULL default '0',
  `EXEC_COUNT` int(10) unsigned default NULL,
  `FAIL_COUNT` int(10) unsigned default NULL,
  `TTIME` int(10) unsigned default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE `ft_stats_summary_ftest` DISABLE KEYS */;
/*!40000 ALTER TABLE `ft_stats_summary_ftest` ENABLE KEYS */;


DROP TABLE IF EXISTS `ft_stats_summary_mptest`;
CREATE TABLE `ft_stats_summary_mptest` (
  `MPTEST_INFO_ID` int(10) unsigned NOT NULL default '0',
  `SITE_NO` smallint(5) unsigned NOT NULL default '0',
  `SPLITLOT_ID` int(10) unsigned NOT NULL default '0',
  `EXEC_COUNT` int(10) unsigned NOT NULL default '0',
  `FAIL_COUNT` int(10) unsigned NOT NULL default '0',
  `MIN_VALUE` float default NULL,
  `MAX_VALUE` float default NULL,
  `SUM` float default NULL,
  `SQUARE_SUM` float default NULL,
  `TTIME` int(10) unsigned default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE `ft_stats_summary_mptest` DISABLE KEYS */;
/*!40000 ALTER TABLE `ft_stats_summary_mptest` ENABLE KEYS */;


DROP TABLE IF EXISTS `ft_stats_summary_ptest`;
CREATE TABLE `ft_stats_summary_ptest` (
  `PTEST_INFO_ID` int(10) unsigned NOT NULL default '0',
  `SITE_NO` smallint(5) unsigned NOT NULL default '0',
  `SPLITLOT_ID` int(10) unsigned NOT NULL default '0',
  `EXEC_COUNT` int(10) unsigned default NULL,
  `FAIL_COUNT` int(10) unsigned default NULL,
  `MIN_VALUE` float default NULL,
  `MAX_VALUE` float default NULL,
  `SUM` float default NULL,
  `SQUARE_SUM` float default NULL,
  `TTIME` int(10) unsigned default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE `ft_stats_summary_ptest` DISABLE KEYS */;
/*!40000 ALTER TABLE `ft_stats_summary_ptest` ENABLE KEYS */;


DROP TABLE IF EXISTS `product`;
CREATE TABLE `product` (
  `PRODUCT_NAME` varchar(255) NOT NULL default '',
  `DESCRIPTION` varchar(1000) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE `product` DISABLE KEYS */;
/*!40000 ALTER TABLE `product` ENABLE KEYS */;


DROP TABLE IF EXISTS `wt_ftest_info`;
CREATE TABLE `wt_ftest_info` (
  `FTEST_INFO_ID` int(10) unsigned NOT NULL auto_increment,
  `SPLITLOT_ID` int(10) unsigned NOT NULL default '0',
  `TNUM` int(10) unsigned NOT NULL default '0',
  `TNAME` varchar(255) NOT NULL default '',
  `TESTSEQ` smallint(5) unsigned default NULL,
  PRIMARY KEY  (`FTEST_INFO_ID`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE `wt_ftest_info` DISABLE KEYS */;
/*!40000 ALTER TABLE `wt_ftest_info` ENABLE KEYS */;


DROP TABLE IF EXISTS `wt_hbin`;
CREATE TABLE `wt_hbin` (
  `HBIN_ID` int(10) unsigned NOT NULL auto_increment,
  `SPLITLOT_ID` int(10) unsigned NOT NULL default '0',
  `HBIN_NO` smallint(5) unsigned NOT NULL default '0',
  `HBIN_NAME` varchar(255) NOT NULL default '',
  `HBIN_CAT` char(1) default NULL,
  `NB_PARTS` int(10) unsigned NOT NULL default '0',
  `NB_RUNS` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`HBIN_ID`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE `wt_hbin` DISABLE KEYS */;
/*!40000 ALTER TABLE `wt_hbin` ENABLE KEYS */;


DROP TABLE IF EXISTS `wt_lot`;
CREATE TABLE `wt_lot` (
  `LOT_ID` varchar(255) NOT NULL default '',
  `PRODUCT_NAME` varchar(255) default NULL,
  `NB_PARTS` int(10) unsigned NOT NULL default '0',
  `NB_PARTS_GOOD` int(10) unsigned NOT NULL default '0',
  `NB_RUNS` int(10) unsigned NOT NULL default '0',
  `NB_RUNS_GOOD` int(10) unsigned NOT NULL default '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE `wt_lot` DISABLE KEYS */;
/*!40000 ALTER TABLE `wt_lot` ENABLE KEYS */;


DROP TABLE IF EXISTS `wt_mptest_info`;
CREATE TABLE `wt_mptest_info` (
  `MPTEST_INFO_ID` int(10) unsigned NOT NULL auto_increment,
  `SPLITLOT_ID` int(10) unsigned NOT NULL default '0',
  `TNUM` int(10) unsigned NOT NULL default '0',
  `TNAME` varchar(255) NOT NULL default '',
  `TPIN` smallint(6) NOT NULL default '0',
  `UNITS` varchar(255) NOT NULL default '',
  `FLAGS` binary(1) NOT NULL default '\0',
  `LL` float default NULL,
  `HL` float default NULL,
  `TESTSEQ` smallint(5) unsigned default NULL,
  PRIMARY KEY  (`MPTEST_INFO_ID`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE `wt_mptest_info` DISABLE KEYS */;
/*!40000 ALTER TABLE `wt_mptest_info` ENABLE KEYS */;


DROP TABLE IF EXISTS `wt_mptest_limits`;
CREATE TABLE `wt_mptest_limits` (
  `MPTEST_INFO_ID` int(10) unsigned NOT NULL default '0',
  `SITE_NO` smallint(5) unsigned NOT NULL default '255',
  `SPLITLOT_ID` int(10) unsigned NOT NULL default '0',
  `LL` float NOT NULL default '0',
  `HL` float NOT NULL default '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE `wt_mptest_limits` DISABLE KEYS */;
/*!40000 ALTER TABLE `wt_mptest_limits` ENABLE KEYS */;


DROP TABLE IF EXISTS `wt_ptest_info`;
CREATE TABLE `wt_ptest_info` (
  `PTEST_INFO_ID` int(10) unsigned NOT NULL auto_increment,
  `SPLITLOT_ID` int(10) unsigned NOT NULL default '0',
  `TNUM` int(10) unsigned NOT NULL default '0',
  `TNAME` varchar(255) NOT NULL default '',
  `UNITS` varchar(255) NOT NULL default '',
  `FLAGS` binary(1) NOT NULL default '\0',
  `LL` float default NULL,
  `HL` float default NULL,
  `TESTSEQ` smallint(5) unsigned default NULL,
  PRIMARY KEY  (`PTEST_INFO_ID`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE `wt_ptest_info` DISABLE KEYS */;
/*!40000 ALTER TABLE `wt_ptest_info` ENABLE KEYS */;


DROP TABLE IF EXISTS `wt_ptest_limits`;
CREATE TABLE `wt_ptest_limits` (
  `PTEST_INFO_ID` int(10) unsigned NOT NULL default '0',
  `SITE_NO` smallint(5) unsigned NOT NULL default '255',
  `SPLITLOT_ID` int(10) unsigned NOT NULL default '0',
  `LL` float NOT NULL default '0',
  `HL` float NOT NULL default '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE `wt_ptest_limits` DISABLE KEYS */;
/*!40000 ALTER TABLE `wt_ptest_limits` ENABLE KEYS */;


DROP TABLE IF EXISTS `wt_result_ftest`;
CREATE TABLE `wt_result_ftest` (
  `FTEST_INFO_ID` int(10) unsigned NOT NULL,
  `RUN_ID` int(10) unsigned NOT NULL default '255',
  `FLAGS` binary(1) NOT NULL default '\0',
  `VECT_NAM` varchar(255) NOT NULL default '',
  `VECT_OFF` smallint(6) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE `wt_result_ftest` DISABLE KEYS */;
/*!40000 ALTER TABLE `wt_result_ftest` ENABLE KEYS */;


DROP TABLE IF EXISTS `wt_result_mptest`;
CREATE TABLE `wt_result_mptest` (
  `MPTEST_INFO_ID` int(10) unsigned NOT NULL default '0',
  `RUN_ID` int(10) unsigned NOT NULL default '255',
  `FLAGS` binary(1) NOT NULL default '\0',
  `VALUE` float NOT NULL default '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE `wt_result_mptest` DISABLE KEYS */;
/*!40000 ALTER TABLE `wt_result_mptest` ENABLE KEYS */;


DROP TABLE IF EXISTS `wt_result_ptest`;
CREATE TABLE `wt_result_ptest` (
  `PTEST_INFO_ID` int(10) unsigned NOT NULL default '0',
  `RUN_ID` int(10) unsigned NOT NULL default '255',
  `FLAGS` binary(1) NOT NULL default '\0',
  `VALUE` float default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE `wt_result_ptest` DISABLE KEYS */;
/*!40000 ALTER TABLE `wt_result_ptest` ENABLE KEYS */;


DROP TABLE IF EXISTS `wt_run`;
CREATE TABLE `wt_run` (
  `RUN_ID` int(10) unsigned NOT NULL auto_increment,
  `SPLITLOT_ID` int(10) unsigned NOT NULL default '0',
  `SITE_NO` smallint(5) unsigned NOT NULL default '255',
  `PART_ID` varchar(255) default NULL,
  `PART_X` smallint(6) default NULL,
  `PART_Y` smallint(6) default NULL,
  `HBIN_NO` smallint(5) unsigned NOT NULL default '0',
  `SBIN_NO` smallint(5) unsigned default NULL,
  `TTIME` int(10) unsigned default NULL,
  `TESTS_EXECUTED` smallint(5) unsigned NOT NULL default '0',
  `TESTS_FAILED` smallint(5) unsigned NOT NULL default '0',
  `FIRSTFAIL_TNUM` int(10) unsigned default NULL,
  `FIRSTFAIL_TNAME` varchar(255) default NULL,
  `RETEST_INDEX` tinyint(3) unsigned NOT NULL default '0',
  PRIMARY KEY  (`RUN_ID`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE `wt_run` DISABLE KEYS */;
/*!40000 ALTER TABLE `wt_run` ENABLE KEYS */;


DROP TABLE IF EXISTS `wt_sbin`;
CREATE TABLE `wt_sbin` (
  `SBIN_ID` int(10) unsigned NOT NULL auto_increment,
  `SPLITLOT_ID` int(10) unsigned NOT NULL default '0',
  `SBIN_NO` smallint(5) unsigned NOT NULL default '0',
  `SBIN_NAME` varchar(255) NOT NULL default '',
  `SBIN_CAT` char(1) default NULL,
  `NB_PARTS` int(10) unsigned NOT NULL default '0',
  `NB_RUNS` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`SBIN_ID`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE `wt_sbin` DISABLE KEYS */;
/*!40000 ALTER TABLE `wt_sbin` ENABLE KEYS */;


DROP TABLE IF EXISTS `wt_splitlot`;
CREATE TABLE `wt_splitlot` (
  `SPLITLOT_ID` int(10) unsigned NOT NULL auto_increment,
  `LOT_ID` varchar(255) NOT NULL default '0',
  `SUBLOT_ID` varchar(255) NOT NULL default '',
  `WAFER_ID` varchar(255) default NULL,
  `SETUP_T` int(10) unsigned NOT NULL default '0',
  `START_T` int(10) unsigned NOT NULL default '0',
  `FINISH_T` int(10) unsigned NOT NULL default '0',
  `STAT_NUM` tinyint(3) unsigned NOT NULL default '0',
  `TESTER_NAME` varchar(255) NOT NULL default '',
  `FLAGS` binary(2) NOT NULL default '\0\0',
  `NB_PARTS` int(10) unsigned NOT NULL default '0',
  `NB_PARTS_GOOD` int(10) unsigned NOT NULL default '0',
  `NB_RUNS` int(10) unsigned NOT NULL default '0',
  `NB_RUNS_GOOD` int(10) unsigned NOT NULL default '0',
  `DATA_ORIGIN` tinyint(3) unsigned NOT NULL default '0',
  `RETEST_INDEX` tinyint(3) unsigned NOT NULL default '0',
  `RETEST_HBINS` varchar(255) default NULL,
  `REWORK_CODE` tinyint(3) unsigned NOT NULL default '0',
  `JOB_NAM` varchar(255) NOT NULL default '',
  `JOB_REV` varchar(255) NOT NULL default '',
  `OPER_NAM` varchar(255) NOT NULL default '',
  `EXEC_TYP` varchar(255) NOT NULL default '',
  `EXEC_VER` varchar(255) NOT NULL default '',
  `TEST_COD` varchar(255) NOT NULL default '',
  `FACIL_ID` varchar(255) NOT NULL default '',
  `TST_TEMP` varchar(255) NOT NULL default '',
  `NB_SITES` tinyint(3) unsigned NOT NULL default '1',
  `HEAD_NUM` tinyint(3) unsigned default NULL,
  `HANDLER_ID` varchar(255) default NULL,
  `LOADBOARD_ID` varchar(255) default NULL,
  `DIB_ID` varchar(255) default NULL,
  `CARD_ID` varchar(255) default NULL,
  `FILE_HOST_ID` int(10) unsigned default '0',
  `FILE_PATH` varchar(255) NOT NULL default '',
  `FILE_NAME` varchar(255) NOT NULL default '',
  PRIMARY KEY  (`SPLITLOT_ID`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE `wt_splitlot` DISABLE KEYS */;
/*!40000 ALTER TABLE `wt_splitlot` ENABLE KEYS */;


DROP TABLE IF EXISTS `wt_stats_hbin`;
CREATE TABLE `wt_stats_hbin` (
  `SPLITLOT_ID` int(10) unsigned NOT NULL default '0',
  `SITE_NO` smallint(5) unsigned NOT NULL default '255',
  `HBIN_ID` int(10) unsigned NOT NULL default '0',
  `NB_PARTS` int(10) unsigned NOT NULL default '0',
  `NB_RUNS` int(10) unsigned NOT NULL default '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE `wt_stats_hbin` DISABLE KEYS */;
/*!40000 ALTER TABLE `wt_stats_hbin` ENABLE KEYS */;


DROP TABLE IF EXISTS `wt_stats_parts`;
CREATE TABLE `wt_stats_parts` (
  `SPLITLOT_ID` int(10) unsigned NOT NULL default '0',
  `SITE_NO` smallint(5) unsigned NOT NULL default '255',
  `NB_RUNS` int(10) unsigned NOT NULL default '0',
  `NB_RUNS_GOOD` int(10) unsigned NOT NULL default '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE `wt_stats_parts` DISABLE KEYS */;
/*!40000 ALTER TABLE `wt_stats_parts` ENABLE KEYS */;


DROP TABLE IF EXISTS `wt_stats_samples_ftest`;
CREATE TABLE `wt_stats_samples_ftest` (
  `FTEST_INFO_ID` int(10) unsigned NOT NULL default '0',
  `SITE_NO` smallint(5) unsigned NOT NULL default '255',
  `SPLITLOT_ID` int(10) unsigned NOT NULL default '0',
  `EXEC_COUNT` int(10) unsigned NOT NULL default '0',
  `FAIL_COUNT` int(10) unsigned default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE `wt_stats_samples_ftest` DISABLE KEYS */;
/*!40000 ALTER TABLE `wt_stats_samples_ftest` ENABLE KEYS */;


DROP TABLE IF EXISTS `wt_stats_samples_mptest`;
CREATE TABLE `wt_stats_samples_mptest` (
  `MPTEST_INFO_ID` int(10) unsigned NOT NULL default '0',
  `SITE_NO` smallint(5) unsigned NOT NULL default '255',
  `SPLITLOT_ID` int(10) unsigned NOT NULL default '0',
  `EXEC_COUNT` int(10) unsigned NOT NULL default '0',
  `FAIL_COUNT` int(10) unsigned default NULL,
  `MIN_VALUE` float default NULL,
  `MAX_VALUE` float default NULL,
  `SUM` float default NULL,
  `SQUARE_SUM` float default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE `wt_stats_samples_mptest` DISABLE KEYS */;
/*!40000 ALTER TABLE `wt_stats_samples_mptest` ENABLE KEYS */;


DROP TABLE IF EXISTS `wt_stats_samples_ptest`;
CREATE TABLE `wt_stats_samples_ptest` (
  `PTEST_INFO_ID` int(10) unsigned NOT NULL default '0',
  `SITE_NO` smallint(5) unsigned NOT NULL default '0',
  `SPLITLOT_ID` int(10) unsigned NOT NULL default '0',
  `EXEC_COUNT` int(10) unsigned NOT NULL default '0',
  `FAIL_COUNT` int(10) unsigned default NULL,
  `MIN_VALUE` float default NULL,
  `MAX_VALUE` float default NULL,
  `SUM` float default NULL,
  `SQUARE_SUM` float default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE `wt_stats_samples_ptest` DISABLE KEYS */;
/*!40000 ALTER TABLE `wt_stats_samples_ptest` ENABLE KEYS */;


DROP TABLE IF EXISTS `wt_stats_sbin`;
CREATE TABLE `wt_stats_sbin` (
  `SPLITLOT_ID` int(10) unsigned NOT NULL default '0',
  `SITE_NO` smallint(5) unsigned NOT NULL default '255',
  `SBIN_ID` int(10) unsigned NOT NULL default '0',
  `NB_PARTS` int(10) unsigned NOT NULL default '0',
  `NB_RUNS` int(10) unsigned NOT NULL default '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE `wt_stats_sbin` DISABLE KEYS */;
/*!40000 ALTER TABLE `wt_stats_sbin` ENABLE KEYS */;


DROP TABLE IF EXISTS `wt_stats_summary_ftest`;
CREATE TABLE `wt_stats_summary_ftest` (
  `FTEST_INFO_ID` int(10) unsigned NOT NULL default '0',
  `SITE_NO` smallint(5) unsigned NOT NULL default '255',
  `SPLITLOT_ID` int(10) unsigned NOT NULL default '0',
  `EXEC_COUNT` int(10) unsigned default NULL,
  `FAIL_COUNT` int(10) unsigned default NULL,
  `TTIME` int(10) unsigned default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE `wt_stats_summary_ftest` DISABLE KEYS */;
/*!40000 ALTER TABLE `wt_stats_summary_ftest` ENABLE KEYS */;


DROP TABLE IF EXISTS `wt_stats_summary_mptest`;
CREATE TABLE `wt_stats_summary_mptest` (
  `MPTEST_INFO_ID` int(10) unsigned NOT NULL default '0',
  `SITE_NO` smallint(5) unsigned NOT NULL default '0',
  `SPLITLOT_ID` int(10) unsigned NOT NULL default '0',
  `EXEC_COUNT` int(10) unsigned NOT NULL default '0',
  `FAIL_COUNT` int(10) unsigned NOT NULL default '0',
  `MIN_VALUE` float default NULL,
  `MAX_VALUE` float default NULL,
  `SUM` float default NULL,
  `SQUARE_SUM` float default NULL,
  `TTIME` int(10) unsigned default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE `wt_stats_summary_mptest` DISABLE KEYS */;
/*!40000 ALTER TABLE `wt_stats_summary_mptest` ENABLE KEYS */;


DROP TABLE IF EXISTS `wt_stats_summary_ptest`;
CREATE TABLE `wt_stats_summary_ptest` (
  `PTEST_INFO_ID` int(10) unsigned NOT NULL default '0',
  `SITE_NO` smallint(5) unsigned NOT NULL default '0',
  `SPLITLOT_ID` int(10) unsigned NOT NULL default '0',
  `EXEC_COUNT` int(10) unsigned default NULL,
  `FAIL_COUNT` int(10) unsigned default NULL,
  `MIN_VALUE` float default NULL,
  `MAX_VALUE` float default NULL,
  `SUM` float default NULL,
  `SQUARE_SUM` float default NULL,
  `TTIME` int(10) unsigned default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

/*!40000 ALTER TABLE `wt_stats_summary_ptest` DISABLE KEYS */;
/*!40000 ALTER TABLE `wt_stats_summary_ptest` ENABLE KEYS */;


DROP TABLE IF EXISTS `wt_wafer_info`;
CREATE TABLE `wt_wafer_info` (
  `SPLITLOT_ID` int(10) unsigned NOT NULL default '0',
  `WAFER_ID` varchar(255) default NULL,
  `FAB_ID` varchar(255) default NULL,
  `FRAME_ID` varchar(255) default NULL,
  `MASK_ID` varchar(255) default NULL,
  `WAFER_SIZE` float default NULL,
  `DIE_HT` float default NULL,
  `DIE_WID` float default NULL,
  `WAFER_UNITS` tinyint(3) unsigned default NULL,
  `WAFER_FLAT` char(1) default NULL,
  `CENTER_X` smallint(5) unsigned default NULL,
  `CENTER_Y` smallint(5) unsigned default NULL,
  `POS_X` char(1) default NULL,
  `POS_Y` char(1) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

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

		SELECT max(Splitlot_ID) FROM ft_splitlot

			WHERE  UNIX_TIMESTAMP() - UNIX_TIMESTAMP(SETUP_T) > (DateEraseEntry*24*60*60*100)

			INTO SplitlotIdForErase;



		IF NOT (SplitlotIdForErase IS NULL)

		THEN

			SELECT max(RUN_ID) FROM ft_run

				WHERE  SPLITLOT_ID <= SplitlotIdForErase

				INTO RunIdForErase;

			# have the splitlot_id and the run_id for delete

	

			SELECT 'TOTAL PURGE for ft_splitlot';

			SELECT SplitlotIdForErase;

			SELECT RunIdForErase;

			DELETE FROM ft_result_ptest WHERE Run_Id <= RunIdForErase;

			DELETE FROM ft_result_ftest WHERE Run_Id <= RunIdForErase;

			DELETE FROM ft_result_mptest WHERE Run_Id <= RunIdForErase;

			DELETE FROM ft_run WHERE Splitlot_Id <= SplitlotIdForErase;

			DELETE FROM ft_stats_summary_ptest WHERE Splitlot_Id <= SplitlotIdForErase;

			DELETE FROM ft_stats_samples_ptest WHERE Splitlot_Id <= SplitlotIdForErase;

			DELETE FROM ft_ptest_limits WHERE Splitlot_Id <= SplitlotIdForErase;

			DELETE FROM ft_ptest_info WHERE Splitlot_Id <= SplitlotIdForErase;

			DELETE FROM ft_stats_summary_ftest WHERE Splitlot_Id <= SplitlotIdForErase;

			DELETE FROM ft_stats_samples_ftest WHERE Splitlot_Id <= SplitlotIdForErase;

			DELETE FROM ft_ftest_info WHERE Splitlot_Id <= SplitlotIdForErase;

			DELETE FROM ft_stats_summary_mptest WHERE Splitlot_Id <= SplitlotIdForErase;

			DELETE FROM ft_stats_samples_mptest WHERE Splitlot_Id <= SplitlotIdForErase;

			DELETE FROM ft_mptest_limits WHERE Splitlot_Id <= SplitlotIdForErase;

			DELETE FROM ft_mptest_info WHERE Splitlot_Id <= SplitlotIdForErase;

			DELETE FROM ft_stats_sbin WHERE Splitlot_Id <= SplitlotIdForErase;

			DELETE FROM ft_stats_hbin WHERE Splitlot_Id <= SplitlotIdForErase;

			DELETE FROM ft_stats_parts WHERE Splitlot_Id <= SplitlotIdForErase;

			DELETE FROM ft_sbin WHERE Splitlot_Id <= SplitlotIdForErase;

			DELETE FROM ft_hbin WHERE Splitlot_Id <= SplitlotIdForErase;

			DELETE FROM ft_splitlot WHERE Splitlot_Id <= SplitlotIdForErase;

		END IF;



		# FOR WT_TABLES

		# Get the Splitlot_Id where to start TotalPurge

		# Get the Run_Id where to start TotalPurge

		SELECT max(Splitlot_ID) FROM wt_splitlot

			WHERE  UNIX_TIMESTAMP() - UNIX_TIMESTAMP(SETUP_T) > (DateEraseEntry*24*60*60*100)

			INTO SplitlotIdForErase;

		IF NOT (SplitlotIdForErase IS NULL)

		THEN

			SELECT max(RUN_ID) FROM wt_run

				WHERE  SPLITLOT_ID <= SplitlotIdForErase

				INTO RunIdForErase;

			# have the splitlot_id and the run_id for delete

	

	                SELECT 'TOTAL PURGE for wt_splitlot';

	                SELECT SplitlotIdForErase;

	                SELECT RunIdForErase;

			DELETE FROM wt_result_ptest WHERE Run_Id <= RunIdForErase;

			DELETE FROM wt_result_ftest WHERE Run_Id <= RunIdForErase;

			DELETE FROM wt_result_mptest WHERE Run_Id <= RunIdForErase;

			DELETE FROM wt_run WHERE Splitlot_Id <= SplitlotIdForErase;

			DELETE FROM wt_stats_summary_ptest WHERE Splitlot_Id <= SplitlotIdForErase;

			DELETE FROM wt_stats_samples_ptest WHERE Splitlot_Id <= SplitlotIdForErase;

			DELETE FROM wt_ptest_limits WHERE Splitlot_Id <= SplitlotIdForErase;

			DELETE FROM wt_ptest_info WHERE Splitlot_Id <= SplitlotIdForErase;

			DELETE FROM wt_stats_summary_ftest WHERE Splitlot_Id <= SplitlotIdForErase;

			DELETE FROM wt_stats_samples_ftest WHERE Splitlot_Id <= SplitlotIdForErase;

			DELETE FROM wt_ftest_info WHERE Splitlot_Id <= SplitlotIdForErase;

			DELETE FROM wt_stats_summary_mptest WHERE Splitlot_Id <= SplitlotIdForErase;

			DELETE FROM wt_stats_samples_mptest WHERE Splitlot_Id <= SplitlotIdForErase;

			DELETE FROM wt_mptest_limits WHERE Splitlot_Id <= SplitlotIdForErase;

			DELETE FROM wt_mptest_info WHERE Splitlot_Id <= SplitlotIdForErase;

			DELETE FROM wt_stats_sbin WHERE Splitlot_Id <= SplitlotIdForErase;

			DELETE FROM wt_stats_hbin WHERE Splitlot_Id <= SplitlotIdForErase;

			DELETE FROM wt_stats_parts WHERE Splitlot_Id <= SplitlotIdForErase;

			DELETE FROM wt_sbin WHERE Splitlot_Id <= SplitlotIdForErase;

			DELETE FROM wt_hbin WHERE Splitlot_Id <= SplitlotIdForErase;

			DELETE FROM wt_wafer_info WHERE Splitlot_Id <= SplitlotIdForErase;

			DELETE FROM wt_splitlot WHERE Splitlot_Id <= SplitlotIdForErase;

		END IF;



		# FOR ET_TABLES

		# Get the Splitlot_Id where to start TotalPurge

		# Get the Run_Id where to start TotalPurge

		SELECT max(Splitlot_ID) FROM et_splitlot

			WHERE  UNIX_TIMESTAMP() - UNIX_TIMESTAMP(SETUP_T) > (DateEraseEntry*24*60*60*100)

			INTO SplitlotIdForErase;



		IF NOT (SplitlotIdForErase IS NULL)

		THEN

			SELECT max(RUN_ID) FROM et_run

				WHERE  SPLITLOT_ID <= SplitlotIdForErase

				INTO RunIdForErase;

			# have the splitlot_id and the run_id for delete

	

	                SELECT 'TOTAL PURGE for et_splitlot';

	                SELECT SplitlotIdForErase;

	                SELECT RunIdForErase;

			DELETE FROM et_result_ptest WHERE Run_Id <= RunIdForErase;

			DELETE FROM et_run WHERE Splitlot_Id <= SplitlotIdForErase;

			DELETE FROM et_stats_summary_ptest WHERE Splitlot_Id <= SplitlotIdForErase;

			DELETE FROM et_stats_samples_ptest WHERE Splitlot_Id <= SplitlotIdForErase;

			DELETE FROM et_ptest_limits WHERE Splitlot_Id <= SplitlotIdForErase;

			DELETE FROM et_ptest_info WHERE Splitlot_Id <= SplitlotIdForErase;

			DELETE FROM et_stats_sbin WHERE Splitlot_Id <= SplitlotIdForErase;

			DELETE FROM et_stats_hbin WHERE Splitlot_Id <= SplitlotIdForErase;

			DELETE FROM et_stats_parts WHERE Splitlot_Id <= SplitlotIdForErase;

			DELETE FROM et_sbin WHERE Splitlot_Id <= SplitlotIdForErase;

			DELETE FROM et_hbin WHERE Splitlot_Id <= SplitlotIdForErase;

			DELETE FROM et_wafer_info WHERE Splitlot_Id <= SplitlotIdForErase;

			DELETE FROM et_splitlot WHERE Splitlot_Id <= SplitlotIdForErase;

		END IF;

	END IF;

	# Samples purge

	# verify if have Samplesge information else skip this step

	IF NOT (DateEraseSamples IS NULL)

	THEN

		# FOR FT_TABLES

		# Get the Splitlot_Id where to start PartialPurge

		# Get the Run_Id where to start PartialPurge

		SELECT max(Splitlot_ID) FROM ft_splitlot

			WHERE  UNIX_TIMESTAMP() - UNIX_TIMESTAMP(SETUP_T) > (DateEraseSamples*24*60*60*100)

			INTO SplitlotIdForErase;



		IF NOT (SplitlotIdForErase IS NULL)

		THEN

			SELECT max(RUN_ID) FROM ft_run

				WHERE  SPLITLOT_ID <= SplitlotIdForErase

				INTO RunIdForErase;

			# have the splitlot_id and the run_id for delete

	

	                SELECT 'PARTIAL PURGE for ft_splitlot';

	                SELECT SplitlotIdForErase;

	                SELECT RunIdForErase;

			DELETE FROM ft_result_ptest WHERE Run_Id <= RunIdForErase;

			DELETE FROM ft_result_ftest WHERE Run_Id <= RunIdForErase;

			DELETE FROM ft_result_mptest WHERE Run_Id <= RunIdForErase;

			DELETE FROM ft_run WHERE Splitlot_Id <= SplitlotIdForErase;

		END IF;



		# FOR WT_TABLES

		# Get the Splitlot_Id where to start PartialPurge

		# Get the Run_Id where to start PartialPurge

		SELECT max(Splitlot_ID) FROM wt_splitlot

			WHERE  UNIX_TIMESTAMP() - UNIX_TIMESTAMP(SETUP_T) > (DateEraseSamples*24*60*60*100)

			INTO SplitlotIdForErase;



		IF NOT (SplitlotIdForErase IS NULL)

		THEN

			SELECT max(RUN_ID) FROM wt_run

				WHERE  SPLITLOT_ID <= SplitlotIdForErase

				INTO RunIdForErase;

			# have the splitlot_id and the run_id for delete

		

	                SELECT 'PARTIAL PURGE for wt_splitlot';

	                SELECT SplitlotIdForErase;

	                SELECT RunIdForErase;

			DELETE FROM wt_result_ptest WHERE Run_Id <= RunIdForErase;

			DELETE FROM wt_result_ftest WHERE Run_Id <= RunIdForErase;

			DELETE FROM wt_result_mptest WHERE Run_Id <= RunIdForErase;

			DELETE FROM wt_run WHERE Splitlot_Id <= SplitlotIdForErase;

		END IF;

	

		# FOR E_TABLES

		# Get the Splitlot_Id where to start PartialPurge

		# Get the Run_Id where to start PartialPurge

		SELECT max(Splitlot_ID) FROM et_splitlot

			WHERE  UNIX_TIMESTAMP() - UNIX_TIMESTAMP(SETUP_T) > (DateEraseSamples*24*60*60*100)

			INTO SplitlotIdForErase;



		IF NOT (SplitlotIdForErase IS NULL)

		THEN

			SELECT max(RUN_ID) FROM et_run

				WHERE  SPLITLOT_ID <= SplitlotIdForErase

				INTO RunIdForErase;

			# have the splitlot_id and the run_id for delete

	

	                SELECT 'PARTIAL PURGE for et_splitlot';

	                SELECT SplitlotIdForErase;

	                SELECT RunIdForErase;

			DELETE FROM et_result_ptest WHERE Run_Id <= RunIdForErase;

			DELETE FROM et_run WHERE Splitlot_Id <= SplitlotIdForErase;

		END IF;	

	END IF;



END $$
DELIMITER ;
DROP PROCEDURE IF EXISTS `gexdb_select_test_procedure`;
DELIMITER $$

CREATE PROCEDURE `gexdb_select_test_procedure`()
BEGIN
	# Get the Splitlot_Id where to start SamplesPurge
	DECLARE SplitlotIdForSelect INT;
	# Get the Run_Id where to start SamplesPurge
	DECLARE MinRunIdForSelect INT;
	DECLARE MaxRunIdForSelect INT;
	# for ft_tables
	SELECT max(Splitlot_ID)
                FROM ft_splitlot INTO SplitlotIdForSelect;
	BEGIN
		DECLARE a INT;
		DECLARE b INT;
		DECLARE c BINARY;
		DECLARE d FLOAT;
		DECLARE MySum INT;
		SELECT max(RUN_ID)
                        FROM ft_run
                        WHERE  SPLITLOT_ID = SplitlotIdForSelect INTO MaxRunIdForSelect;
		SELECT min(RUN_ID)
                        FROM ft_run
                        WHERE  SPLITLOT_ID = SplitlotIdForSelect INTO MinRunIdForSelect;

		# have the splitlot_id and the run_id for selection

                SELECT 'SELECTION';
                SELECT SplitlotIdForSelect;
                SELECT MinRunIdForSelect;
                SELECT MaxRunIdForSelect;

		BEGIN

			DECLARE not_found BOOLEAN DEFAULT false;

			DECLARE cur10 CURSOR FOR SELECT RUN_ID FROM ft_result_ptest WHERE Run_Id <= MaxRunIdForSelect AND Run_Id >= MinRunIdForSelect;

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
