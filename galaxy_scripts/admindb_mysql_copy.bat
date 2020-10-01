@echo off

REM script to transfer a db from a MySQL server to another one WITHOUT writing .sql file(s). 

echo Starting  export: > Export.log
time /T >> Export.log
echo ... >> Export.log

REM Test : export to samba file. Exple : \\Jupiter\Jupiter_Shared\db.sql
REM mysqldump --no-defaults -u gexdb_new -pgexadmin_new --skip-set-charset --skip-opt --hex_blob --comments --disable-keys --extended-insert --no-create-info --quick  gexdb_new wt_ptest_results > \\Jupiter\Jupiter_Shared\gexdb_new_ptest_results.sql

REM --add-drop-table :   Add a DROP TABLE before each create.
REM --create-options : Inclut toutes les options spécifiques MySQL de création de table dans les commandes CREATE TABLE
REM --no-defaults	: Don't read default options from any option file.
REM --no-create-info : Don't write table creation info. N'écrit pas les informations de création de table (la requête CREATE TABLE).
REM --no-create-db : CREATE DATABASE /*!32312 IF NOT EXISTS*/ db_name; ne sera pas ajouté dans l'export. Sinon, la ligne ci-dessus sera ajoutée, si l'une des options --databases ou --all-databases ont été activée.
REM --skip-opt : ?

set GEXDB_SOURCE=ym_admin_db
set GEXDB_TARGET=ym_admin_db

set SOURCE_HOST=192.168.0.13
set TARGET_HOST=localhost

set SOURCE_USER=ym_admin_db
set SOURCE_PASSWORD=arthur66
set TARGET_PASSWORD=root

set CREATEDB=--verbose

REM mysql create schema ym_admin_db

rem ym_nodes ym_nodes_options ym_settings
REM --add-drop-table  --no-create-info --create-options
mysqldump --no-defaults  -u %SOURCE_USER% -p%SOURCE_PASSWORD% --host=%SOURCE_HOST% --verbose --skip-set-charset --skip-opt --hex_blob  --comments  --no-create-info --disable-keys  --extended-insert %GEXDB_SOURCE% ym_databases_options ym_tasks_options | mysql -u root -p%TARGET_PASSWORD% --host=%TARGET_HOST% --compress --database=%GEXDB_TARGET%  

mysqldump --no-defaults  -u %SOURCE_USER% -p%SOURCE_PASSWORD% --host=%SOURCE_HOST% --verbose --skip-set-charset --skip-opt --hex_blob  --comments  --no-create-info --disable-keys  --extended-insert %GEXDB_SOURCE% ym_databases ym_tasks | mysql -u root -p%TARGET_PASSWORD% --host=%TARGET_HOST% --compress --database=%GEXDB_TARGET%  

REM mysqldump --no-defaults -u %SOURCE_USER% -p%SOURCE_PASSWORD% --host=%SOURCE_HOST% --verbose --skip-set-charset --skip-opt --hex_blob --create-options --comments --disable-keys %CREATEDB%  --extended-insert --quick %GEXDB_SOURCE% gexdb_debug | mysql -u root -parthur66 --host=%TARGET_HOST% --compress --database=%GEXDB_TARGET%  

REM mysqldump --no-defaults -u %SOURCE_USER% -p%SOURCE_PASSWORD% --host=%SOURCE_HOST% --verbose --skip-set-charset --skip-opt --hex_blob --create-options --comments --disable-keys %CREATEDB%  --extended-insert --quick %GEXDB_SOURCE% gexdb_log | mysql -u root -parthur66 --host=%TARGET_HOST% --compress --database=%GEXDB_TARGET%  

REM echo sending wt_run ...
REM mysqldump --no-defaults -u %SOURCE_USER% -p%SOURCE_PASSWORD% --host=%SOURCE_HOST% --verbose --skip-set-charset --skip-opt --hex_blob --create-options --comments --disable-keys   --extended-insert --quick %GEXDB_SOURCE% wt_run | mysql -u root -parthur66 --host=%TARGET_HOST% --compress --database=%GEXDB_TARGET% 

REM mysqldump --no-defaults -u %SOURCE_USER% -p%SOURCE_PASSWORD% --host=%SOURCE_HOST% --verbose --skip-set-charset --skip-opt --hex_blob --create-options --comments --disable-keys   --extended-insert --quick %GEXDB_SOURCE% wt_product_hbin | mysql -u root -parthur66 --host=%TARGET_HOST% --compress --database=%GEXDB_TARGET% 

REM mysqldump --no-defaults -u %SOURCE_USER% -p%SOURCE_PASSWORD% --host=%SOURCE_HOST% --verbose --skip-set-charset --skip-opt --hex_blob --create-options --comments --disable-keys   --extended-insert --quick %GEXDB_SOURCE% wt_product_sbin | mysql -u root -parthur66 --host=%TARGET_HOST% --compress --database=%GEXDB_TARGET% 

REM echo sending wt_ptest_results (the biggest)...
REM mysqldump --no-defaults -u %SOURCE_USER% -p%SOURCE_PASSWORD% --host=%SOURCE_HOST%  --verbose --skip-set-charset --skip-opt --hex_blob --create-options --comments --disable-keys --extended-insert --quick %GEXDB_SOURCE% wt_ptest_results | mysql -u root -parthur66 --host=%TARGET_HOST% --compress --database=%GEXDB_TARGET%

REM mysqldump --no-defaults -u %SOURCE_USER% -p%SOURCE_PASSWORD% --host=%SOURCE_HOST%  --verbose --skip-set-charset --skip-opt --hex_blob --create-options --comments --disable-keys --extended-insert --quick %GEXDB_SOURCE% wt_consolidated_wafer | mysql -u root -parthur66 --host=%TARGET_HOST% --compress --database=%GEXDB_TARGET%

REM wt_*
REM mysqldump --no-defaults -u %SOURCE_USER% -p%SOURCE_PASSWORD% --host=%SOURCE_HOST% --verbose --skip-set-charset  --skip-opt --hex_blob --create-options --comments --disable-keys --no-create-db --extended-insert --quick  %GEXDB_SOURCE% incremental_update product file_host wt_metadata_link  wt_sbl wt_wyr wt_sya_set wt_wyr_format  wt_lot wt_lot_hbin wt_lot_sbin wt_wafer_info wt_wafer_hbin wt_wafer_sbin wt_prod_alarm wt_splitlot wt_parts_stats_samples wt_parts_stats_summary wt_hbin wt_hbin_stats_samples wt_hbin_stats_summary wt_sbin wt_sbin_stats_samples wt_sbin_stats_summary wt_ptest_info wt_ptest_limits wt_ptest_stats_samples wt_ptest_stats_summary wt_mptest_info wt_mptest_limits wt_mptest_results wt_mptest_stats_samples wt_mptest_stats_summary wt_ftest_info wt_ftest_results wt_ftest_stats_samples wt_ftest_stats_summary | mysql -u root -parthur66 --host=%TARGET_HOST% --compress --database=%GEXDB_TARGET%

REM echo Final test tables
REM mysqldump --no-defaults -u %SOURCE_USER% -p%SOURCE_PASSWORD% --host=%SOURCE_HOST% --verbose --skip-set-charset  --skip-opt --hex_blob --create-options --comments --disable-keys --no-create-db --extended-insert --quick  %GEXDB_SOURCE% ft_metadata_link  ft_sbl ft_wyr ft_sya_set ft_wyr_format  ft_lot ft_lot_hbin ft_lot_sbin ft_prod_alarm ft_splitlot ft_parts_stats_samples ft_parts_stats_summary ft_hbin ft_hbin_stats_samples ft_hbin_stats_summary ft_sbin ft_sbin_stats_samples ft_sbin_stats_summary ft_ptest_info ft_ptest_limits ft_ptest_stats_samples ft_ptest_stats_summary ft_mptest_info ft_mptest_limits ft_mptest_results ft_mptest_stats_samples ft_mptest_stats_summary ft_ftest_info ft_ftest_results ft_ftest_stats_samples ft_ftest_stats_summary | mysql -u root -parthur66 --host=%TARGET_HOST% --compress --database=%GEXDB_TARGET%

REM echo ft_run...
REM mysqldump --no-defaults -u %SOURCE_USER% -p%SOURCE_PASSWORD% --host=%SOURCE_HOST% --verbose --skip-set-charset --skip-opt --hex_blob --create-options --comments --disable-keys --no-create-db --extended-insert --quick %GEXDB_SOURCE% ft_run | mysql -u root -parthur66 --host=%TARGET_HOST% --compress --database=%GEXDB_TARGET% 

REM echo ft_ptest_results...
REM mysqldump --no-defaults -u %SOURCE_USER% -p%SOURCE_PASSWORD% --host=%SOURCE_HOST%  --verbose --skip-set-charset --skip-opt --hex_blob --create-options --comments --disable-keys --no-create-db  --extended-insert --quick %GEXDB_SOURCE% ft_ptest_results | mysql -u root -parthur66 --host=%TARGET_HOST% --compress --database=%GEXDB_TARGET%

REM ft_consolidated_lot ft_die_tracking        
REM mysqldump --no-defaults -u %SOURCE_USER% -p%SOURCE_PASSWORD% --host=%SOURCE_HOST%  --verbose --skip-set-charset --skip-opt --hex_blob --create-options --comments --disable-keys --no-create-db  --extended-insert --quick %GEXDB_SOURCE% ft_consolidated_lot ft_die_tracking | mysql -u root -parthur66 --host=%TARGET_HOST% --compress --database=%GEXDB_TARGET%

REM mysqldump --no-defaults -u %SOURCE_USER% -p%SOURCE_PASSWORD% --host=%SOURCE_HOST%  --verbose --skip-set-charset --skip-opt --hex_blob --create-options --comments --disable-keys --no-create-db  --extended-insert --quick %GEXDB_SOURCE% ft_metadata_mapping | mysql -u root -parthur66 --host=%TARGET_HOST% --compress --database=%GEXDB_TARGET%

REM mysqldump --no-defaults -u %SOURCE_USER% -p%SOURCE_PASSWORD% --host=%SOURCE_HOST%  --verbose --skip-set-charset --skip-opt --hex_blob --create-options --comments --disable-keys --no-create-db  --extended-insert --quick %GEXDB_SOURCE% ft_product_hbin | mysql -u root -parthur66 --host=%TARGET_HOST% --compress --database=%GEXDB_TARGET%



REM custom tables
REM mysqldump --no-defaults -u %SOURCE_USER% -p%SOURCE_PASSWORD% --host=%SOURCE_HOST% --verbose --skip-set-charset --hex_blob --create-options --comments --disable-keys --no-create-db --extended-insert --quick  %GEXDB_SOURCE% vy_promis_info vy_promis_info_debug | mysql -u root -parthur66 --host=%TARGET_HOST% --compress --database=%GEXDB_TARGET%

REM mysqldump --no-defaults -u %SOURCE_USER% -p%SOURCE_PASSWORD% --host=%SOURCE_HOST% --verbose --skip-set-charset --hex_blob --create-options --comments --disable-keys --no-create-db --extended-insert --quick  %GEXDB_SOURCE% vy_gdpw | mysql -u root -parthur66 --host=%TARGET_HOST% --compress --database=%GEXDB_TARGET%

REM pause
REM exit

REM e-test
REM mysqldump --no-defaults -u %SOURCE_USER% -p%SOURCE_PASSWORD% --host=%SOURCE_HOST% --verbose --add-drop-table --skip-set-charset  --hex_blob --create-options --comments --disable-keys --no-create-db --extended-insert --quick  %GEXDB_SOURCE% et_hbin et_lot et_lot_hbin et_lot_sbin et_metadata_link et_metadata_mapping et_prod_alarm et_ptest_info et_ptest_results et_ptest_stats et_run et_sbin et_sbl et_splitlot et_sya_set et_wafer_hbin et_wafer_info et_wafer_sbin et_wyr et_wyr_format | mysql -u root -parthur66 --host=%TARGET_HOST% --verbose --compress --database=%GEXDB_TARGET%

REM  et_product_hbin et_product_sbin
REM mysqldump --no-defaults -u %SOURCE_USER% -p%SOURCE_PASSWORD% --host=%SOURCE_HOST% --verbose --add-drop-table --skip-set-charset  --hex_blob --create-options --comments --disable-keys --no-create-db --extended-insert --quick  %GEXDB_SOURCE%  et_product_hbin et_product_sbin | mysql -u root -parthur66 --host=%TARGET_HOST% --verbose --compress --database=%GEXDB_TARGET%


REM stored proc ??? does not work on MySQL 5.0 ...
REM mysqldump --no-defaults -u gexdb_new -pgexadmin_new --verbose --skip-set-charset --add-drop-table --skip-opt --hex_blob --create-options --comments --disable-keys --no-create-db --extended-insert --quick gexdb_new ET_FILEARCHIVE_SETTINGS | mysql -u root -parthur66 --host=192.168.1.26 --compress --database=gexdb_vy 

REM views  ??? does not work on MySQL 5.0 ...
REM mysqldump --no-defaults -u gexdb_new -pgexadmin_new --verbose --skip-set-charset --add-drop-table --skip-opt --hex_blob --create-options --comments --disable-keys --no-create-db --extended-insert --quick gexdb_new metadata_distincts_view test_view wt_consildated_fields wt_consolidated_wafer_fields wt_splitlot_view | mysql -u root -parthur66 --host=192.168.1.26 --compress --database=gexdb_vy 

echo export finished >> Export.log
time /T >> Export.log
pause
