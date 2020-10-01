DROP USER 'gexdb'@'%';
DROP USER 'gexdb_user'@'%';
DROP USER 'gexdb_rep'@'%';

GRANT ALL ON gexdb.* TO 'gexdb'@'%' IDENTIFIED BY 'gexadmin';
GRANT FILE ON *.* TO 'gexdb'@'%' IDENTIFIED BY 'gexadmin';
GRANT SELECT, SHOW VIEW, CREATE VIEW, CREATE TEMPORARY TABLES 
	ON gexdb.* TO 'gexdb_user'@'%' IDENTIFIED BY 'gexuser';
GRANT REPLICATION SLAVE, REPLICATION CLIENT ON *.* 
	TO 'gexdb_rep'@'%' IDENTIFIED BY 'gexreplication';