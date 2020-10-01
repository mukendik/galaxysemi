-- MySQL Administrator dump 1.4
--
-- ------------------------------------------------------
-- Server version	5.1.40-community


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;

/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;


--
-- Create schema ym_admin_db
--
USE ym_admin_db;

CREATE TABLE `ym_settings` (
  `field` varchar(256) NOT NULL DEFAULT '',
  `value` varchar(1024),
  PRIMARY KEY (field)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

INSERT INTO `ym_settings` (`field`,`value`) VALUES
 ('DB_VERSION_NB','6.1'),
 ('DB_BUILD_NB','27'),
 ('DB_VERSION_NAME','Yield-Man Administration Server'),
 ('DB_OPTIONLEVEL','5'),
 ('DB_MIN_BEFORE_DISCONNECT','5'),
 ('BI_SERVER',NULL),
 ('LDAP_SERVER',NULL);

CREATE TABLE `ym_databases` (
  `database_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `node_id` int(10) unsigned NOT NULL DEFAULT '0',
  `name` varchar(256) NOT NULL DEFAULT '',
  `description` varchar(256) DEFAULT NULL,
  `plugin_name` varchar(256) DEFAULT NULL,
  `plugin_file` varchar(256) DEFAULT NULL,
  `plugin_version` varchar(256) DEFAULT NULL,
  `host` varchar(256) DEFAULT NULL,
  `port` mediumint(8) unsigned DEFAULT NULL,
  `driver` varchar(256) DEFAULT NULL COMMENT 'MySql Oracle',
  `schema` varchar(256) DEFAULT NULL,
  `database` varchar(256) DEFAULT NULL,
  `user_name` varchar(256) DEFAULT NULL,
  `user_pwd` varchar(256) DEFAULT NULL,
  `admin_name` varchar(256) DEFAULT NULL,
  `admin_pwd` varchar(256) DEFAULT NULL,
  `incremental_updates` tinyint(3) unsigned DEFAULT NULL,
  `creation_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `expiration_date` datetime DEFAULT NULL,
  `type` varchar(30) DEFAULT NULL,
  `last_update` datetime DEFAULT NULL,
  `mutex` varchar(256) DEFAULT NULL,
  PRIMARY KEY (database_id)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `ym_databases_options` (
  `database_id` int(10) unsigned NOT NULL DEFAULT '0',
  `field` varchar(256) NOT NULL DEFAULT '',
  `value` varchar(1024) NOT NULL DEFAULT '',
  PRIMARY KEY (database_id,field)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `ym_nodes` (
  `node_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `cpu` varchar(256) NOT NULL DEFAULT '' COMMENT 'signature',
  `host_name` varchar(256) DEFAULT NULL,
  `host_id` varchar(256) DEFAULT NULL COMMENT 'signature',
  `os` varchar(256) DEFAULT NULL,
  `os_login` varchar(256) DEFAULT NULL,
  `gex_server_profile` varchar(1024) DEFAULT NULL COMMENT 'signature',
  `type` varchar(256) DEFAULT NULL,
  `name` varchar(256) DEFAULT NULL,
  `status` varchar(256) DEFAULT NULL,
  `last_update` datetime DEFAULT NULL,
  `mutex` varchar(256) DEFAULT NULL,
  PRIMARY KEY (node_id)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `ym_nodes_options` (
  `node_id` int(10) unsigned NOT NULL DEFAULT '0',
  `field` varchar(256) NOT NULL DEFAULT '',
  `value` varchar(1024),
  PRIMARY KEY (node_id,field)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `ym_tasks` (
  `task_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `node_id` int(11) DEFAULT NULL,
  `user_id` int(11) DEFAULT NULL,
  `group_id` int(11) DEFAULT NULL,
  `database_id` int(11) DEFAULT NULL,
  `name` varchar(256) NOT NULL DEFAULT '',
  `type` tinyint(3) DEFAULT NULL,
  `enabled` tinyint(3) NOT NULL DEFAULT '1',
  `permisions` mediumint(8) NOT NULL DEFAULT '0',
  `creation_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `expiration_date` datetime DEFAULT NULL,
  `last_update` datetime DEFAULT NULL,
  `mutex` varchar(256) DEFAULT NULL,
  PRIMARY KEY (task_id)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `ym_tasks_options` (
  `task_id` int(10) unsigned NOT NULL DEFAULT '0',
  `field` varchar(256) NOT NULL DEFAULT '',
  `value` TEXT,
  PRIMARY KEY (task_id,field)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `ym_users` (
  `user_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `group_id` int(10) unsigned DEFAULT NULL COMMENT 'primary group',
  `login` varchar(256) NOT NULL,
  `pwd` varchar(256) DEFAULT NULL,
  `name` varchar(256) NOT NULL COMMENT 'name of the user / description of the account',
  `email` varchar(256) DEFAULT NULL COMMENT 'email of the user / list of emails associated',
  `type` tinyint(3) unsigned NOT NULL COMMENT 'MasterAdmin / GroupAdmin / User',
  `os_login` varchar(256) DEFAULT NULL COMMENT 'last os_login from windows or unix',
  `profile_id` int(10) unsigned DEFAULT NULL COMMENT 'deafult profile',
  `creation_date` datetime NOT NULL,
  `expiration_date` datetime DEFAULT NULL COMMENT 'null if any',
  `last_access` datetime DEFAULT NULL COMMENT 'last connection',
  `last_update` datetime DEFAULT NULL,
  `mutex` varchar(256) DEFAULT NULL,
  PRIMARY KEY (user_id)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `ym_users_options` (
  `user_id` int(10) unsigned NOT NULL DEFAULT '0',
  `field` varchar(256) NOT NULL DEFAULT '',
  `value` varchar(1024),
  PRIMARY KEY (user_id,field)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `ym_users_profiles` (
  `profile_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `user_id` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Owner',
  `name` varchar(256) NOT NULL DEFAULT '',
  `description` varchar(256) DEFAULT NULL,
  `os_login` varchar(256) DEFAULT NULL COMMENT 'first os_login from windows or unix',
  `permisions` mediumint(8) unsigned DEFAULT NULL,
  `script_name` varchar(256) DEFAULT NULL,
  `script_content` mediumtext,
  `creation_date` datetime NOT NULL,
  `last_update` datetime DEFAULT NULL,
  PRIMARY KEY (profile_id)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `ym_events` (
  `creation_time` datetime NOT NULL,
  `event_id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `link` bigint(20) unsigned DEFAULT NULL,
  `category` varchar(256) NOT NULL,
  `type` varchar(256) NOT NULL,
  `start_time` datetime DEFAULT NULL,
  `duration` int(10) unsigned DEFAULT '0',
  `size` int(10) unsigned DEFAULT '0',
  `node_id` int(10) unsigned DEFAULT NULL,
  `user_id` int(10) unsigned DEFAULT NULL,
  `task_id` int(10) unsigned DEFAULT NULL,
  `database_id` int(10) unsigned DEFAULT NULL,
  `command` varchar(1024) DEFAULT NULL,
  `status` varchar(256) DEFAULT NULL,
  `summary` mediumtext,
  `comment` varchar(1024) DEFAULT NULL,
  PRIMARY KEY (event_id,creation_time)
  ) ENGINE=InnoDB DEFAULT CHARSET=latin1
  PARTITION BY RANGE (TO_DAYS(creation_time))
 (PARTITION LastPart VALUES LESS THAN (MAXVALUE));

CREATE TABLE `ym_actions` (
  `creation_time` datetime NOT NULL,
  `action_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `category` varchar(256) NOT NULL,
  `type` varchar(256) NOT NULL,
  `start_time` datetime DEFAULT NULL,
  `node_list` varchar(256) DEFAULT '|',
  `mutex` varchar(256) DEFAULT NULL,
  `task_id` int(10) unsigned DEFAULT NULL,
  `database_id` int(10) unsigned DEFAULT NULL,
  `status` varchar(256) DEFAULT NULL,
  `command` TEXT DEFAULT NULL,
   PRIMARY KEY (node_list,action_id),
   KEY ymactions_action (action_id)
  ) ENGINE=InnoDB DEFAULT CHARSET=latin1;


CREATE TABLE `da_galaxy` (
  `idda_galaxy` int(11) NOT NULL,
  `session_id` varchar(255) DEFAULT NULL,
  `directory` mediumtext,
  `checksum` int(10) DEFAULT NULL,
  `last_update` datetime DEFAULT NULL,
  PRIMARY KEY (`idda_galaxy`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

DROP TABLE IF EXISTS `ym_sya` ;

CREATE TABLE `ym_sya` (
  `task_id` int(10) unsigned NOT NULL AUTO_INCREMENT COMMENT 'Unique identifier of the sya task',
  `testing_stage` varchar(45) NOT NULL COMMENT 'testing stage for which this sya task is set',
  `product_regexp` varchar(1024) NOT NULL COMMENT 'regular expression used to resolve at runtime the product list on which the spm task must be executed',
  `monitored_item_type`char(1) NOT NULL COMMENT 'type of the binning to monitor ("S" for soft, "H" for hard binnings)',
  `monitored_item_regexp` varchar(1024) NOT NULL COMMENT 'regular expression used to generate at config time the bin list on which the sya task must be executed',
  `test_flow` varchar(45) NOT NULL COMMENT 'name of the test flow for which the sya limits are being computed and checked',
  `consolidation_type` varchar(45) NOT NULL  COMMENT '"1" to compute and check limits on consolidated data, "0" to compute and check limits on raw data',
  `consolidation_aggregation_level` varchar(45) NOT NULL COMMENT '"test_insertion" or "test_flow" consolidation aggregation level',
  `consolidation_name_regexp` varchar(255) DEFAULT NULL COMMENT 'regular expression used to define the consolidation name(s) used to compute and check limits',
  `site_merge_mode` varchar(45) NOT NULL COMMENT 'per_site (split), merged_sites (merged) or both (both) processing mode of the test sites',
  `stats_to_monitor` varchar(255) NOT NULL COMMENT 'list of statistics to be monitored separated by a pipe. For SYA this value is always "ratio"',
  `min_lots` smallint(5) unsigned NOT NULL COMMENT 'minimum number of lots necessary to compute the sya limits',
  `min_datapoints` smallint(5) unsigned NOT NULL COMMENT 'minimum number of datapoints used to compute the sya limits',
  `use_gross_die` tinyint(1) NOT NULL COMMENT 'whether the gross_die count should be used instead of the nb_parts when computing a ratio',
  `threshold` double DEFAULT NULL COMMENT 'threshold that must be reached, if not null, to trigger the alarm',
  `default_algorithm` varchar(45) NOT NULL COMMENT 'algorithm used by default for calculating the sya limits',
  `remove_outliers` tinyint(1) NOT NULL COMMENT 'whether the outliers should be removed during the limits computation',
  `validity_period` smallint(5) unsigned NOT NULL COMMENT 'number of days before the created sya is invalid',
  `days_before_expiration` smallint(5) NOT NULL COMMENT 'number of days before the end of the validity period where warning emails and/or recompute will be triggered',
  `send_email_before_expiration` tinyint(1) NOT NULL COMMENT 'whether an email must be sent when the limit before expiration triggers',
  `auto_recompute` tinyint(1) NOT NULL COMMENT 'whether the limits must be automatically recomputed when the limit before expiration triggers',
  `auto_recompute_method` varchar(45) NOT NULL COMMENT 'method used for limit recomputation: "duplicate"=use previous limits, "recompute"=compute new ones, "duplicateIfRecomputeFails"=duplicate previous limits if recomputation failed',
  `auto_recompute_period` smallint(5) NOT NULL COMMENT 'number of days that must be used to automatically recompute the limits',
  `email_format` varchar(45) DEFAULT NULL COMMENT 'format of the alert email',
  `email_from` varchar(255) DEFAULT NULL COMMENT '"from" value of the alert email',
  `email_report_type` varchar(45) DEFAULT NULL COMMENT 'report type of the alert email',
  `emails` varchar(1024) DEFAULT NULL COMMENT 'recipients of the alert email',
  PRIMARY KEY (`task_id`)
) ENGINE=InnoDB AUTO_INCREMENT=6 DEFAULT CHARSET=utf8 COMMENT='describes an sya task';

DROP TABLE IF EXISTS `ym_sya_default_params` ;

CREATE TABLE `ym_sya_default_params` (
  `task_id` int(10) unsigned NOT NULL COMMENT 'foreign key to the sya task identifier',
  `criticity_level` tinyint(3) unsigned NOT NULL COMMENT 'criticity level to which this default parameter is applied',
  `param_name` varchar(15) NOT NULL COMMENT 'name of the parameter (ex: N)',
  `param_value` float DEFAULT NULL COMMENT 'value of the parameter',
  PRIMARY KEY (`task_id`,`criticity_level`,`param_name`),
  KEY `fk_ym_sya_default_params_ym_sya1_idx` (`task_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='default parameter to use when computing the stat limits of a new sya version';

DROP TABLE IF EXISTS `ym_sya_filters` ;

CREATE TABLE `ym_sya_filters` (
  `task_id` int(10) unsigned NOT NULL COMMENT 'Unique identifier of the sya task',
  `field` varchar(255) NOT NULL COMMENT 'field used to filter the datapoints',
  `value` varchar(1024) NOT NULL COMMENT 'value used to filter the datapoints',
  PRIMARY KEY (`task_id`,`field`),
  KEY `ymsyafilters_field` (`field`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='describes the custom filters on a sya task';

DROP TABLE IF EXISTS `ym_sya_version` ;

CREATE TABLE `ym_sya_version` (
  `task_id` int(10) unsigned NOT NULL COMMENT 'foreign key to the sya task owning this version',
  `version_id` smallint(5) unsigned NOT NULL COMMENT 'sya version identifier',
  `draft_version` tinyint(1) NOT NULL DEFAULT '0' COMMENT 'Flags whether this version is a draft in progress or a production version usable for sya run',
  `version_label` varchar(255) DEFAULT NULL COMMENT 'user defined label for this version',
  `matched_products` varchar(1024) NOT NULL COMMENT 'comma separated list of products used to compute the limits on that sya version',
  `creation_date` datetime NOT NULL COMMENT 'sya version creation date',
  `start_date` datetime NOT NULL COMMENT 'date from which the sya version can be run',
  `expiration_date` datetime NOT NULL COMMENT 'date until which the sya version can be run',
  `expiration_warning_date` datetime DEFAULT NULL COMMENT 'date at which an expiration email must be sent',
  `expiration_warning_done` tinyint(1) DEFAULT NULL COMMENT '0 = no warning have been sent yet, 1 = a warning has been sent, 2 = a 24h warning has been sent',
  `computation_fromdate` datetime NOT NULL COMMENT 'start of the time range used to compute the version limits',
  `computation_todate` datetime NOT NULL COMMENT 'end of the time range used to compute the version limits',
  PRIMARY KEY (`task_id`,`version_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='version of a sya task implementing a given set of limits';

DROP TABLE IF EXISTS `ym_sya_monitored_item` ;

CREATE TABLE `ym_sya_monitored_item` (
  `task_id` int(10) unsigned NOT NULL COMMENT 'foreign key to the sya task monitoring this item',
  `monitored_item_id` int(10) unsigned NOT NULL COMMENT 'bin identifier',
  `monitored_item_type` varchar(1) NOT NULL COMMENT 'H (hard) or S (soft) binning type',
  `monitored_item_num` varchar(255) NOT NULL COMMENT 'bin number. Use "-1" for the monitoring item which is the actual yield',
  `monitored_item_name` varchar(255) NOT NULL COMMENT 'bin name. Use "yield" for the monitoring item which is the actual yield',
  `monitored_item_unit` varchar(255) DEFAULT NULL COMMENT 'The unit of the monitored item. Typically: "%" for bin monitoring',
  `monitored_item_scale` tinyint(3) DEFAULT NULL COMMENT 'unused for SYA, defined for SYA/SPM queries consistency',
  `monitored_item_cat` varchar(255) NOT NULL COMMENT 'P (pass) or F (fail) bin category',
  PRIMARY KEY (`task_id`,`monitored_item_id`),
  UNIQUE KEY `UNIQUE` (`task_id`,`monitored_item_type`,`monitored_item_num`,`monitored_item_name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='identification of a item monitored by the sya task. For instance this item in a bin in SYA';

DROP TABLE IF EXISTS `ym_sya_limit` ;

CREATE TABLE `ym_sya_limit` (
  `task_id` int(10) unsigned NOT NULL COMMENT 'foreign key to the sya task owning this limit',
  `version_id` smallint(5) unsigned NOT NULL COMMENT 'foreign key to the sya version owning this limit',
  `limit_id` int(10) unsigned NOT NULL COMMENT 'Limit identifier',
  `site_no` smallint(5) NOT NULL COMMENT 'test site number, -1 for merge',
  `criticity_level` tinyint(3) unsigned NOT NULL COMMENT 'limits criticity level: ex. 1 (standard) or 2 (critical)',
  `stat_name` varchar(45) NOT NULL COMMENT 'name of the stat being monitored (for instance, SYA monitors only the Yield of a bin)',
  `has_unit` tinyint(1) NOT NULL COMMENT 'whether the value has a unit (same as the test)',
  `ll_enabled` tinyint(1) NOT NULL COMMENT 'whether the low limit is enabled',
  `ll` double DEFAULT NULL COMMENT 'low limit value, NULL if not computed',
  `hl_enabled` tinyint(1) NOT NULL COMMENT 'whether the high limit is enabled',
  `hl` double DEFAULT NULL COMMENT 'high limit value, NULL if not computed',
  `mean` double DEFAULT NULL COMMENT 'mean used to compute the limits',
  `sigma` double DEFAULT NULL COMMENT 'sigma used to compute the limits',
  `median` double DEFAULT NULL COMMENT 'median used to compute the limits',
  `q1` double DEFAULT NULL COMMENT 'Q1 used to compute the limits',
  `q3` double DEFAULT NULL COMMENT 'Q3 used to compute the limits',
  `percent_n` double DEFAULT NULL COMMENT 'N percentile used to compute the limits',
  `percent_100_min_n` double DEFAULT NULL COMMENT '100-N percentile used to compute the limits',
  `algorithm` varchar(45) NOT NULL COMMENT 'algorithm used to compute that stat limits',
  `computation_datapoints` int(10) unsigned DEFAULT NULL COMMENT 'number of datapoints used during the limits iterative computation process, NULL if not computed',
  `computation_outliers` int(10) unsigned DEFAULT NULL COMMENT 'number of outliers put aside during the limits iterative computation process, NULL if not computed',
  `enabled` tinyint(1) NOT NULL COMMENT 'whether the limits are enabled at all',
  `recompute` tinyint(1) NOT NULL COMMENT 'whether the stat need to be recomputed after an algo/N update',
  PRIMARY KEY (`task_id`,`version_id`,`limit_id`),
  UNIQUE KEY `unique_composite` (`task_id`,`version_id`,`limit_id`,`criticity_level`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='represents a single set of limits to use while monitoring a bin, at a given criticity level';

DROP TABLE IF EXISTS `ym_sya_limit_monitored_item` ;

CREATE TABLE `ym_sya_limit_monitored_item` (
    `task_id` int(10) unsigned NOT NULL COMMENT 'foreign key to the sya task owning the limit',
    `version_id` smallint(5) unsigned NOT NULL COMMENT 'foreign key to the sya version owning the limit',
    `limit_id` int(10) unsigned NOT NULL COMMENT 'foreign key to the limit',
    `monitored_item_id` int(10) unsigned NOT NULL COMMENT 'foreign key to the monitored item',
    PRIMARY KEY (`task_id`,`version_id`,`limit_id`,`monitored_item_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='describes the links between a limit and its monitored items';

DROP TABLE IF EXISTS `ym_sya_limit_param` ;

CREATE TABLE `ym_sya_limit_param` (
  `task_id` int(10) unsigned NOT NULL COMMENT 'foreign key to the sya task',
  `version_id` smallint(5) unsigned NOT NULL COMMENT 'foreign key to the sya version',
  `limit_id` int(10) unsigned NOT NULL COMMENT 'foreign key to the sya limit set for which the parameter is used to compute the limits',
  `param_name` varchar(15) NOT NULL COMMENT 'name of the parameter (ex: N)',
  `param_value` float DEFAULT NULL COMMENT 'value of the parameter',
  PRIMARY KEY (`task_id`,`version_id`,`limit_id`,`param_name`),
  KEY `fk_ym_sya_stat_param_ym_sya_parameter1_idx` (`limit_id`,`task_id`,`version_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='parameter used when computing a limit set';

DROP TABLE IF EXISTS `ym_sya_log` ;

CREATE TABLE `ym_sya_log` (
  `log_id` int(10) unsigned NOT NULL AUTO_INCREMENT COMMENT 'Unique identifier of the log entry',
  `task_id` int(10) unsigned NOT NULL COMMENT 'foreign key to the sya identifier',
  `version_id` smallint(5) unsigned NOT NULL COMMENT 'foreign key to the sya version identifier',
  `execution_date` datetime NOT NULL COMMENT 'sya task execution date',
  `execution_type` varchar(1) NOT NULL COMMENT 'M (manual), I (insertion) or T (trigger) sya activation',
  `matched_products` varchar(1024) DEFAULT NULL COMMENT 'list of the products on which that particular sya execution resolved',
  `matched_lots` varchar(1024) DEFAULT NULL COMMENT 'list of the lots on which that particular sya execution resolved',
  `matched_sublots` text COMMENT 'list of the (Lots|sublots/wafers) on which that particular sya execution resolved',
  `matched_wafers` text COMMENT 'list of the (Lots|sublots/wafers) on which that particular sya execution resolved',
  `nb_parts` int(10) unsigned DEFAULT NULL COMMENT 'number of parts tested according to the resolved products/lots/sublots',
  `nb_alarms` int(10) unsigned DEFAULT NULL COMMENT 'how many splitlot level alarms this sya run raised',
  `status` varchar(45) DEFAULT NULL COMMENT 'status of the execution ie PASS, FAIL, IN PROGRESS...',
  `summary` text COMMENT 'summary of the execution',
  PRIMARY KEY (`log_id`),
  KEY `fk_ym_sya_log_ym_sya_stat_idx` (`task_id`,`version_id`,`execution_date`),
  KEY `ymsyalog_status` (`status`)
) ENGINE=InnoDB AUTO_INCREMENT=76 DEFAULT CHARSET=utf8 COMMENT='logging of a single sya execution';

DROP TABLE IF EXISTS `ym_sya_alarm` ;

CREATE TABLE `ym_sya_alarm` (
  `log_id` int(10) unsigned NOT NULL COMMENT 'foreign key to the log entry',
  `task_id` int(10) unsigned NOT NULL COMMENT 'foreign key to the sya task',
  `version_id` smallint(5) unsigned NOT NULL COMMENT 'foreign key to the sya version',
  `limit_id` int(10) NOT NULL COMMENT 'foreign key to the sya limit set',
  `execution_date` datetime NOT NULL COMMENT 'sya task execution date',
  `lot_id` varchar(255) NOT NULL COMMENT 'lot id for which the alarm was raised',
  `sublot_id` varchar(255) NOT NULL COMMENT 'sublot id for which the alarm was raised',
  `wafer_id` varchar(255) NOT NULL COMMENT 'wafer id for which the alarm was raised',
  `splitlot_id` int(10) unsigned NOT NULL COMMENT 'first splitlot id inserted in the wafer/sublot that raised the alarm',
  `product_name` varchar(255) NOT NULL COMMENT 'product name for which the alarm was raised',
  `monitored_item_id` int(10) unsigned NOT NULL COMMENT 'foreign key to the monitored item which raised the alarm',
  `site_no` smallint(5) NOT NULL COMMENT 'test site number, -1 for merge',
  `exec_count` mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'number of parts in the binning that raised the alarm',
  `fail_count` mediumint(8) unsigned DEFAULT NULL COMMENT 'equals 0 if the bin is a Pass one, exec_count otherwise.\nFor the global yield, equals the sum of the fail bins',
  `criticity_level` tinyint(3) unsigned NOT NULL COMMENT 'limits criticity level: ex. 1 (standard) or 2 (critical)',
  `stat_name` varchar(45) NOT NULL COMMENT 'name of the stat being tested',
  `ll` double DEFAULT NULL COMMENT 'low limit value, NULL if no limit',
  `hl` double DEFAULT NULL COMMENT 'high limit value, NULL if no limit',
  `outlier_value` double NOT NULL COMMENT 'value of the detected outlier',
  PRIMARY KEY (`log_id`,`limit_id`,`execution_date`,`lot_id`,`sublot_id`,`wafer_id`,`splitlot_id`,`monitored_item_id`),
  KEY `fk_ym_sya_alarm_ym_sya_stat_idx` (`task_id`,`version_id`,`limit_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='alarms raised by sya executions';

DROP TABLE IF EXISTS `ym_spm` ;

CREATE TABLE `ym_spm` (
  `task_id` int(10) unsigned NOT NULL AUTO_INCREMENT COMMENT 'Unique identifier of the spm task',
  `testing_stage` varchar(45) NOT NULL COMMENT 'testing stage for which this spm task is set',
  `product_regexp` varchar(1024) NOT NULL COMMENT 'regular expression used to resolve at runtime the product list on which the spm task must be executed',
  `monitored_item_type`char(1) NOT NULL COMMENT 'type of the test to monitor ("P" for parametric, "M" for multiparametric tests)',
  `monitored_item_regexp` varchar(1024) NOT NULL COMMENT 'regular expression used to generate at config time the test list on which the spm task must be executed',
  `test_flow` varchar(45) NOT NULL COMMENT 'name of the test flow for which the sya limits are being computed and checked',
  `consolidation_type` varchar(45) NOT NULL  COMMENT '"1" to compute and check limits on consolidated data, "0" to compute and check limits on raw data',
  `consolidation_aggregation_level` varchar(45) NOT NULL COMMENT '"test_insertion" or "test_flow" consolidation aggregation level',
  `consolidation_name_regexp` varchar(255) DEFAULT NULL COMMENT 'regular expression used to define the consolidation name(s) used to compute and check limits',
  `site_merge_mode` varchar(45) NOT NULL COMMENT 'per_site (split), merged_sites (merged) or both (both) processing mode of the test sites',
  `stats_to_monitor` varchar(255) NOT NULL COMMENT 'list of statistics to be monitored separated by a pipe (ie: mean|sigma)',
  `min_lots` smallint(5) unsigned NOT NULL COMMENT 'minimum number of lots necessary to compute the spm limits',
  `min_datapoints` smallint(5) unsigned NOT NULL COMMENT 'minimum number of datapoints used to compute the spm limits',
  `use_gross_die` tinyint(1) NOT NULL COMMENT 'not used for SPM',
  `threshold` double DEFAULT NULL COMMENT 'threshold that must be reached, if not null, to trigger the alarm',
  `default_algorithm` varchar(45) NOT NULL COMMENT 'algorithm used by default for calculating the spm limits',
  `remove_outliers` tinyint(1) NOT NULL COMMENT 'whether the outliers should be removed during the limits computation',
  `validity_period` smallint(5) unsigned NOT NULL COMMENT 'number of days before the created spm is invalid',
  `days_before_expiration` smallint(5) NOT NULL COMMENT 'number of days before the end of the validity period where warning emails and/or recompute will be triggered',
  `send_email_before_expiration` tinyint(1) NOT NULL COMMENT 'whether an email must be sent when the limit before expiration triggers',
  `auto_recompute` tinyint(1) NOT NULL COMMENT 'whether the limits must be automatically recomputed when the limit before expiration triggers',
  `auto_recompute_method` varchar(45) NOT NULL COMMENT 'method used for limit recomputation: "duplicate"=use previous limits, "recompute"=compute new ones, "duplicateIfRecomputeFails"=duplicate previous limits if recomputation failed',
  `auto_recompute_period` smallint(5) NOT NULL COMMENT 'number of days that must be used to automatically recompute the limits',
  `email_format` varchar(45) DEFAULT NULL COMMENT 'format of the alert email',
  `email_from` varchar(255) DEFAULT NULL COMMENT '"from" value of the alert email',
  `email_report_type` varchar(45) DEFAULT NULL COMMENT 'report type of the alert email',
  `emails` varchar(1024) DEFAULT NULL COMMENT 'recipients of the alert email',
  PRIMARY KEY (`task_id`)
) ENGINE=InnoDB AUTO_INCREMENT=6 DEFAULT CHARSET=utf8 COMMENT='describes an spm task';

DROP TABLE IF EXISTS `ym_spm_default_params` ;

CREATE TABLE `ym_spm_default_params` (
  `task_id` int(10) unsigned NOT NULL COMMENT 'foreign key to the spm task',
  `criticity_level` tinyint(3) unsigned NOT NULL COMMENT 'criticity level to which this default parameter is applied',
  `param_name` varchar(15) NOT NULL COMMENT 'name of the parameter (ex: N)',
  `param_value` float DEFAULT NULL COMMENT 'value of the parameter',
  PRIMARY KEY (`task_id`,`criticity_level`,`param_name`),
  KEY `fk_ym_spm_default_params_ym_spm1_idx` (`task_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='default parameter to use when computing the stat limits of a new spm version';

DROP TABLE IF EXISTS `ym_spm_filters` ;

CREATE TABLE `ym_spm_filters` (
  `task_id` int(10) unsigned NOT NULL COMMENT 'foreign key to the spm task',
  `field` varchar(255) NOT NULL COMMENT 'field used to filter the datapoints',
  `value` varchar(1024) NOT NULL COMMENT 'value used to filter the datapoints',
  PRIMARY KEY (`task_id`,`field`),
  KEY `ymspmfilters_field` (`field`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='describes the custom filters on a spm task';

DROP TABLE IF EXISTS `ym_spm_version` ;

CREATE TABLE `ym_spm_version` (
  `task_id` int(10) unsigned NOT NULL COMMENT 'foreign key to the spm owning this version',
  `version_id` smallint(5) unsigned NOT NULL COMMENT 'spm version unique identifier',
  `draft_version` tinyint(1) NOT NULL DEFAULT '0' COMMENT 'Flags whether this version is a draft in progress or a production version usable for spm run',
  `version_label` varchar(255) DEFAULT NULL COMMENT 'user defined label for this version',
  `matched_products` varchar(1024) NOT NULL COMMENT 'comma separated list of products used to compute the limits on that spm version',
  `creation_date` datetime NOT NULL COMMENT 'spm version creation date',
  `start_date` datetime NOT NULL COMMENT 'date from which the spm version can be run',
  `expiration_date` datetime NOT NULL COMMENT 'date until which the spm version can be run',
  `expiration_warning_date` datetime DEFAULT NULL COMMENT 'date at which an expiration email must be sent',
  `expiration_warning_done` tinyint(1) DEFAULT NULL COMMENT '0 = no warning have been sent yet, 1 = a warning has been sent, 2 = a 24h warning has been sent',
  `computation_fromdate` datetime NOT NULL COMMENT 'start of the time range used to compute the version limits',
  `computation_todate` datetime NOT NULL COMMENT 'end of the time range used to compute the version limits',
  PRIMARY KEY (`task_id`,`version_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='version of a spm task implementing the limits to check on a given test perimeter';

DROP TABLE IF EXISTS `ym_spm_monitored_item` ;

CREATE TABLE `ym_spm_monitored_item` (
  `task_id` int(10) unsigned NOT NULL COMMENT 'foreign key to the spm identifier to which this spm test is attached',
  `monitored_item_id` int(10) unsigned NOT NULL COMMENT 'spm test identifier',
  `monitored_item_type` varchar(1) NOT NULL COMMENT 'P (parametric) or M (multiparametric) test type',
  `monitored_item_num` varchar(255) NOT NULL COMMENT 'test number',
  `monitored_item_name` varchar(255) NOT NULL COMMENT 'test name',
  `monitored_item_unit` varchar(255) DEFAULT NULL COMMENT 'test unit',
  `monitored_item_scale` tinyint(3) DEFAULT NULL COMMENT 'test scale for display',
  `monitored_item_cat` varchar(255) DEFAULT NULL COMMENT 'unused for SPM, defined for SYA/SPM queries consistency',
  PRIMARY KEY (`task_id`,`monitored_item_id`),
  UNIQUE KEY `UNIQUE` (`task_id`,`monitored_item_type`,`monitored_item_num`,`monitored_item_name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='identification of a test -using its type, number and name- under spm watch';

DROP TABLE IF EXISTS `ym_spm_limit` ;

CREATE TABLE `ym_spm_limit` (
  `task_id` int(10) unsigned NOT NULL COMMENT 'foreign key to the spm task',
  `version_id` smallint(5) unsigned NOT NULL COMMENT 'foreign key to the spm version',
  `limit_id` int(10) unsigned NOT NULL COMMENT 'limit identifier',
  `site_no` smallint(5) NOT NULL COMMENT 'test site number, -1 for merge',
  `criticity_level` tinyint(3) unsigned NOT NULL COMMENT 'limits criticity level: ex. 1 (standard) or 2 (critical)',
  `stat_name` varchar(45) NOT NULL COMMENT 'name of the stat being monitored',
  `has_unit` tinyint(1) NOT NULL COMMENT 'whether the value has a unit (same as the test)',
  `ll_enabled` tinyint(1) NOT NULL COMMENT 'whether the low limit is enabled',
  `ll` double DEFAULT NULL COMMENT 'low limit value, NULL if not computed',
  `hl_enabled` tinyint(1) NOT NULL COMMENT 'whether the high limit is enabled',
  `hl` double DEFAULT NULL COMMENT 'high limit value, NULL if not computed',
  `mean` double DEFAULT NULL COMMENT 'mean used to compute the limits',
  `sigma` double DEFAULT NULL COMMENT 'sigma used to compute the limits',
  `median` double DEFAULT NULL COMMENT 'median used to compute the limits',
  `q1` double DEFAULT NULL COMMENT 'Q1 used to compute the limits',
  `q3` double DEFAULT NULL COMMENT 'Q3 used to compute the limits',
  `percent_n` double DEFAULT NULL COMMENT 'N percentile used to compute the limits',
  `percent_100_min_n` double DEFAULT NULL COMMENT '100-N percentile used to compute the limits',
  `algorithm` varchar(45) NOT NULL COMMENT 'algorithm used to compute that stat limits',
  `computation_datapoints` int(10) unsigned DEFAULT NULL COMMENT 'number of datapoints used during the limits iterative computation process, NULL if not computed',
  `computation_outliers` int(10) unsigned DEFAULT NULL COMMENT 'number of outliers put aside during the limits iterative computation process, NULL if not computed',
  `enabled` tinyint(1) NOT NULL COMMENT 'whether the limits are enabled at all',
  `recompute` tinyint(1) NOT NULL COMMENT 'whether the stat need to be recomputed after an algo/N update',
  PRIMARY KEY (`task_id`,`version_id`,`limit_id`),
  UNIQUE KEY `unique_composite` (`task_id`,`version_id`,`limit_id`,`site_no`,`criticity_level`,`stat_name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='represents a single set of limits to use while monitoring a test, for a given stat, using a given site number, at a given criticity level';

DROP TABLE IF EXISTS `ym_spm_limit_monitored_item` ;

CREATE TABLE `ym_spm_limit_monitored_item` (
    `task_id` int(10) unsigned NOT NULL COMMENT 'foreign key to the spm task owning the limit',
    `version_id` smallint(5) unsigned NOT NULL COMMENT 'foreign key to the spm version owning the limit',
    `limit_id` int(10) unsigned NOT NULL COMMENT 'foreign key to the limit',
    `monitored_item_id` int(10) unsigned NOT NULL COMMENT 'foreign key to the monitored item',
    PRIMARY KEY (`task_id`,`version_id`,`limit_id`,`monitored_item_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='describes the links between a limit and its monitored items';

DROP TABLE IF EXISTS `ym_spm_limit_param` ;

CREATE TABLE `ym_spm_limit_param` (
  `task_id` int(10) unsigned NOT NULL COMMENT 'foreign key to the spm task',
  `version_id` smallint(5) unsigned NOT NULL COMMENT 'foreign key to the spm version',
  `limit_id` int(10) unsigned NOT NULL COMMENT 'foreign key to the spm limits set for which the parameter is used to compute the limits',
  `param_name` varchar(15) NOT NULL COMMENT 'name of the parameter (ex: N)',
  `param_value` float DEFAULT NULL COMMENT 'value of the parameter',
  PRIMARY KEY (`task_id`,`version_id`,`limit_id`,`param_name`),
  KEY `fk_ym_spm_stat_param_ym_spm_parameter1_idx` (`limit_id`,`task_id`,`version_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='parameter used when computing a limit set';

DROP TABLE IF EXISTS `ym_spm_log` ;

CREATE TABLE `ym_spm_log` (
  `log_id` int(10) unsigned NOT NULL AUTO_INCREMENT COMMENT 'Unique identifier of the log entry',
  `task_id` int(10) unsigned NOT NULL COMMENT 'foreign key to the spm task',
  `version_id` smallint(5) unsigned NOT NULL COMMENT 'foreign key to the spm version',
  `execution_date` datetime NOT NULL COMMENT 'spm task execution date',
  `execution_type` varchar(1) NOT NULL COMMENT 'M (manual), I (insertion) or T (trigger) spm activation',
  `matched_products` varchar(1024) DEFAULT NULL COMMENT 'list of the products on which that particular spm execution resolved',
  `matched_lots` varchar(1024) DEFAULT NULL COMMENT 'list of the lots on which that particular spm execution resolved',
  `matched_sublots` text COMMENT 'list of the (Lots|sublots/wafers) on which that particular spm execution resolved',
  `matched_wafers` text COMMENT 'list of the (Lots|sublots/wafers) on which that particular spm execution resolved',
  `nb_parts` int(10) unsigned DEFAULT NULL COMMENT 'number of parts tested according to the resolved products/lots/sublots',
  `nb_alarms` int(10) unsigned DEFAULT NULL COMMENT 'how many splitlot level alarms this spm run raised',
  `status` varchar(45) DEFAULT NULL COMMENT 'status of the execution ie PASS, FAIL, IN PROGRESS...',
  `summary` text COMMENT 'summary of the execution',
  PRIMARY KEY (`log_id`),
  KEY `fk_ym_spm_log_ym_spm_stat_idx` (`task_id`,`version_id`,`execution_date`),
  KEY `ymspmlog_status` (`status`)
) ENGINE=InnoDB AUTO_INCREMENT=76 DEFAULT CHARSET=utf8 COMMENT='logging of a single spm execution';

DROP TABLE IF EXISTS `ym_spm_alarm` ;

CREATE TABLE `ym_spm_alarm` (
  `log_id` int(10) unsigned NOT NULL COMMENT 'foreign key to the log',
  `task_id` int(10) unsigned NOT NULL COMMENT 'foreign key to the spm task',
  `version_id` smallint(5) unsigned NOT NULL COMMENT 'foreign key to the spm version',
  `limit_id` int(10) NOT NULL COMMENT 'foreign key to the limit set',
  `execution_date` datetime NOT NULL COMMENT 'spm task execution date',
  `lot_id` varchar(255) NOT NULL COMMENT 'lot id for which the alarm was raised',
  `sublot_id` varchar(255) NOT NULL COMMENT 'sublot id for which the alarm was raised',
  `wafer_id` varchar(255) NOT NULL COMMENT 'wafer id for which the alarm was raised',
  `splitlot_id` int(10) unsigned NOT NULL COMMENT 'first splitlot id inserted in the wafer/sublot that raised the alarm',
  `product_name` varchar(255) NOT NULL COMMENT 'product name for which the alarm was raised',
  `monitored_item_id` int(10) unsigned NOT NULL COMMENT 'foreign key to the monitored item which raised the alarm',
  `site_no` smallint(5) NOT NULL COMMENT 'test site number, -1 for merge',
  `exec_count` mediumint(8) unsigned NOT NULL DEFAULT '0' COMMENT 'number of parts tested by the test which raised the alarm',
  `fail_count` mediumint(8) unsigned DEFAULT NULL COMMENT 'number of parts which were classified as bad by the test which raised the alarm',
  `criticity_level` tinyint(3) unsigned NOT NULL COMMENT 'stat limits criticity level: ex. 1 (standard) or 2 (critical)',
  `stat_name` varchar(45) NOT NULL COMMENT 'name of the stat being tested',
  `ll` double DEFAULT NULL COMMENT 'low limit value, NULL if no limit',
  `hl` double DEFAULT NULL COMMENT 'high limit value, NULL if no limit',
  `outlier_value` double NOT NULL COMMENT 'value of the detected outlier',
  PRIMARY KEY (`log_id`,`limit_id`,`execution_date`,`lot_id`,`sublot_id`,`wafer_id`,`splitlot_id`,`monitored_item_id`),
  KEY `fk_ym_spm_alarm_ym_spm_stat_idx` (`task_id`,`version_id`,`limit_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='alarms raised by spm executions';

DELIMITER ;;
CREATE EVENT `event_add_partition` ON SCHEDULE EVERY 1 DAY STARTS '2012-01-01 00:00:00' ON COMPLETION NOT PRESERVE ENABLE DO BEGIN
  -- New partition name (DayAfter)
  DECLARE new_partition CHAR(32) DEFAULT
    CONCAT( 'p',         DATE_FORMAT( DATE_ADD( CURDATE(), INTERVAL 1 DAY ), '%Y%m%d' )         );
  -- New max value (DayAfter+1)
  DECLARE max_day INTEGER DEFAULT TO_DAYS( CURDATE() ) + 2;
  DECLARE status_msg VARCHAR(1024);
  DECLARE status_value VARCHAR(10);
  DECLARE CONTINUE HANDLER FOR 1517
    BEGIN
      set @status_msg = CONCAT('Duplicate partition name ',new_partition);
      set @status_value = 'FAIL';
    END;
  DECLARE CONTINUE HANDLER FOR SQLEXCEPTION
    BEGIN
      set @status_msg = 'Error occurred';
      set @status_value = 'FAIL';
    END;

   -- Update the Events table
  SET @StartTime = now();
  SET @status_value = 'PASS';
  SET @status_msg = CONCAT('New partition name ',new_partition);
  INSERT INTO ym_events
    VALUES(now(),0,null,'UPDATE','ADMINISTRATION',@StartTime,0,0,null,null,null,null,
    'Execute=event_add_partition','START','Event scheduler running','');
  SET @event_id = LAST_INSERT_ID();

  -- Prepare split LastPartition
  SET @stmt = CONCAT('ALTER TABLE ym_events REORGANIZE PARTITION LastPart INTO (PARTITION ',
    new_partition,         ' VALUES LESS THAN (',          max_day,          '),
    PARTITION LastPart VALUES LESS THAN MAXVALUE)');
  PREPARE stmt FROM @stmt;  -- Execute and split the Last partition
  EXECUTE stmt;

  -- Update the Events table
  INSERT INTO ym_events
    VALUES(now(),0,@event_id,'UPDATE','ADMINISTRATION',@StartTime,0,0,null,null,null,null,
    'Execute=event_add_partition',@status_value,@status_msg,'');
END;;
DELIMITER ;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
