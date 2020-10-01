--
-- Table structure for table global_options
--

DROP TABLE IF EXISTS global_info;

CREATE TABLE global_info (
  db_version_name varchar(255) NOT NULL COMMENT 'commercial name of the database version',
  db_version_nb smallint(5) NOT NULL COMMENT 'technical number of the database version',
  db_version_build smallint(5) NOT NULL COMMENT 'build number of the database version',
  db_status varchar(255) DEFAULT NULL COMMENT 'current state of the database, stores the tasks that needs to be done if the database is not ready',
  db_type varchar(255) DEFAULT NULL COMMENT 'stores a ciphered description of the database type (adr db)',
  PRIMARY KEY (db_version_build)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='global information about the database version and status';

INSERT INTO global_info VALUES ('ADR V7.03 B78 (MySQL)', 703, 78, '', NULL);
>>>>>>> c690bcc0a1... GCORE-14130
