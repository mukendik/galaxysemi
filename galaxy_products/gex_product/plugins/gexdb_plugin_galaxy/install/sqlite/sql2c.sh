#!/bin/bash
# ---------------------------------------------------------------------------- #
# sql2c.sh
# ---------------------------------------------------------------------------- #
# Alignment example for TDR b35 and sqlite b3
# ---------------------------------------------------------------------------- #
# 1) sqlite ==> TDR
# ---------------------------------------------------------------------------- #
# obsolete :
#     http://sourceforge.net/projects/fsqlf/
#     ~/fsqlf/linux/fsqlf gs_tdr_b2.sql gs_tdr_b3.sql
#     vi gs_tdr_b3.sql  # suppr first and last '"'
# cp gs_tdr_b3.sql ~/tmp/gs_tdr_b3.sql
# vi ~/tmp/gs_tdr_b3.sql
# INTEGER CHECK ( >=0 ) DEFAULT '0'  ==> INTEGER unsigned DEFAULT '0'
# suppr CONSTRAINT FOREIGN KEY
# suppr '' in PRIMARY KEY
# 1,$s/splitlot_id INTEGER/splitlot_id int(10)/
# 1,$s/ptest_info_id INTEGER/ptest_info_id smallint(5)/
# 1,$s/run_id INTEGER/run_id mediumint(7)/
# 1,$s/exec_count INTEGER/exec_count mediumint(8) unsigned NOT NULL DEFAULT '0'/
# 1,$s/fail_count INTEGER/fail_count mediumint(8) unsigned NOT NULL DEFAULT '0'/
# 1,$s/INTEGER/int(10)/
# 1,$s/ CHAR / char(1) /
#
# grep INDEX ~/tmp/gs_tdr_b3.sql
# update :
# bool GexDbPlugin_Galaxy::UpdateDb_UpdateIndexes(QStringList lstIndexesToCheck)
# cp gexdb_mysql_install_b34.sql gexdb_mysql_install_b35.sql
# diff gs_tdr_b2.sql gs_tdr_b3.sql
# vi gexdb_mysql_install_b35.sql
# vi ~/tmp/gs_tdr_b3.sql
# insert new tables/fields from ~/tmp/gs_tdr_b3.sql according to the diff output
# without CREATE INDEX and add "ENGINE=InnoDB DEFAULT CHARSET=latin1"
# cp gexdb_mysql_update_b??_to_b??.sql gexdb_mysql_update_b34_to_b35.sql
# vi gexdb_mysql_update_b34_to_b35.sql  # CREATE / add_column_if_not_exists
#
# Test db update and db creation :
# a) test db update
# cd ~/prod/gex-prod-qa-master
# BRANCH=master source ~/.gex-prod-conf.sh  # (MYSQL_PWD)
# gex-prod-specs/createDB.sh\
#  gex-prod-tests/common/database_b34.sql gexdb_qa194 gexdb_user_qa194
# gex -WYM  # (~/prod/gex-prod-specs/gex.sh)
# update gexdb_qa194
# check indexes :
# mysql -h 127.0.0.1 -u root gexdb_qa194 -e\
#  "show create table ft_ptest_rollinglimits"
# b) test db creation + indexes
#
# ---------------------------------------------------------------------------- #
# 2) TDR ==> sqlite
# ---------------------------------------------------------------------------- #
# https://gist.github.com/esperlu/943776
# git clone --depth 1 https://gist.github.com/943776.git mysql2sqlite
# cd mysql2sqlite
# chmod 755 mysql2sqlite.sh
# BRANCH=master source ~/.gex-prod-conf.sh
# ./mysql2sqlite.sh --no-data -h 127.0.0.1 -u root gexdb_qa194 |\
#  sqlite3 gs_tdr_b3.db
# sqlite3 gs_tdr_b3.db '.schema' >gs_tdr_for_diff.sql
# git diff gs_tdr_for_diff.sql
# vi gs_tdr_b3.sql  # update according to the diff output
# sql2c.sh gs_tdr_b3.sql gs_tdr_b3_c.sql
# cd ~/prod/gex-prod-master/galaxy_products/gex_product/gex-tester/gtl/gtl-core
# vi gtl_output_sqlite.c  # gs_tdr_b2_c.sql ==> gs_tdr_b3_c.sql
#
# Test with unit test :
# cd ~/prod/gex-prod-master/galaxy_products/gex_product/gex-tester/gtl/gtl-core
# touch gtl_output_sqlite.c
# compile  # (~/prod/gex-prod-specs/compile.sh)
# cd ../gtl-core-dynlib
# compile
# cd ~/prod/gex-prod-master/galaxy_unit_tests
# ./make-unit-tests.sh --branch=master --timeout=600 --log=/dev/stdout --debug\
#  --color galaxy_unit_tests/gtl-sqlite-schema/gtl-sqlite-schema
# ---------------------------------------------------------------------------- #

if [ -z "$2" ]; then
    echo "Usage: `basename $0` <sql-file> <out-file>"
    exit 0
fi
sqlfile=$1
outfile=$2
cat $sqlfile |\
 tr '\n' ' ' |\
 sed 's/  */ /g' |\
 sed 's/\(.*\)/"\1"/'\
 >$outfile

exit 0
