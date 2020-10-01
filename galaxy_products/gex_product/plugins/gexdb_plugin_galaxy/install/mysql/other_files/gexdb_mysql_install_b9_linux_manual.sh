mysql --user=root -p --execute="CREATE DATABASE gexdb_lin2"
mysql --user=root -p --execute="GRANT ALL ON gexdb_lin2.* TO 'gexdb_lin2'@'%%' IDENTIFIED BY 'gexadmin_lin2'"
mysql --user=root -p --execute="GRANT FILE ON *.* TO 'gexdb_lin2'@'%%' IDENTIFIED BY 'gexadmin_lin2'"
mysql --user=root -p --execute="GRANT SELECT, SHOW VIEW, CREATE VIEW, CREATE TEMPORARY TABLES ON gexdb_lin2.* TO 'gexdb_user_lin2'@'%%' IDENTIFIED BY 'gexuser_lin2'"
mysql -u gexdb_lin2 -p gexdb_lin2 < gexdb_mysql_install_b9.sql

