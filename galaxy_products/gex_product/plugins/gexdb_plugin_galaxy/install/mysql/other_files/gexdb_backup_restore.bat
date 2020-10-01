@echo OFF
SET CallErrorMessage=

SetLocal
cls
SET ProgramName=gexdb_backup_restore
SET EchoOn=
SET PauseMode=
SET NoDisplay=
SET a=
SET TempGalaxyFile=%ProgramName%TempGalaxyFile.tmp

:AddCallParams
IF {%1}=={} 		goto FinAddCallParams
IF {%1}=={-echo} 	SET EchoOn=%1 & SHIFT & goto AddCallParams
IF {%1}=={-pause} 	SET PauseMode=%1 & SHIFT & goto AddCallParams
IF {%1}=={-nodisplay} 	SET NoDisplay=%1 & SHIFT & goto AddCallParams
@echo Usage :
@echo Usage : -echo 		: display all lines
@echo Usage : -pause 		: add some pause
@echo Usage : -nodisplay 	: for monitoring
goto Exit

:FinAddCallParams
IF DEFINED NoDisplay SET EchoOn= & SET PauseMode=

IF DEFINED EchoON @echo ON

REM this program restore the GexDb with the last backup sql file and all bin-log

SET TempGalaxyFile=%ProgramName%TempGalaxyFile.tmp
del /q "%TempGalaxyFile%" 2>NUL

@echo Read MySql and GexDb configuration file ...
call gexdb_readconfig.bat %EchoOn% ReadMySqlIni ReadGexDbIni
IF DEFINED CallErrorMessage			SET ErrorMessage=%CallErrorMessage% & goto Error

IF NOT DEFINED MySqlIni 			SET ErrorMessage= cannot find my.ini in %CD%\..\ & goto Error
IF NOT DEFINED datadir 				SET ErrorMessage= no datadir defined in my.ini & goto Error
IF NOT DEFINED GexDbIni 			SET ErrorMessage= cannot find GexDb.ini in %CD% & goto Error
IF NOT DEFINED GexAdminName 			SET ErrorMessage= GexDb Administrator not defined 	& goto Error
IF NOT DEFINED GexUserName 			SET ErrorMessage= GexDb User not defined 		& goto Error
IF NOT DEFINED GexReplicationName 		SET ErrorMessage= GexDb Replication not defined 	& goto Error
IF NOT DEFINED GexBackupDir 			SET ErrorMessage= GexBackupDir not defined	& goto Error

@echo Restore GexDb from last Backup ...
IF NOT EXIST "%GexBackupDir%" SET ErrorMessage= BackupDir %GexBackupDir% doesn't exist	& goto Error

REM SEARCH THE LAST BACKUP DIR USED FOR STORED DUMP AND BINLOG FILES
REM LASTBACKUPDIR IS LIKE 20061224
SET LastBackupDir=
FOR /f %%a IN ('dir /B /A:D "%GexBackupDir%20*" /O') DO SET LastBackupDir=%%a
IF NOT DEFINED LastBackupDir 			SET ErrorMessage= Incremental BackupDir doesn't exist in %GexBackupDir% & goto Error

SET MySqlUser=--user=%GexAdminName% --password=%GexAdminPwd% %GexDataBaseName%
REM TEST CONNECTION
mysql %MySqlUser% --execute="show databases" 2>"%TempGalaxyFile%" >NUL
IF EXIST "%TempGalaxyFile%" SET /p a=<"%TempGalaxyFile%"
IF DEFINED a SET ErrorMessage= GexDb_admin connection fail: %a%	& goto Error

FOR /f %%a IN ('dir /B "%BackupDir%%LastBackupDir%\*.zip" /O') DO unzip -o "%BackupDir%%LastBackupDir%\%%a" -d "%BackupDir%%LastBackupDir%\"

REM LOCK ALL TABLES FROM THE GEXDB DATABASE
mysql %MySqlUser% --execute="show tables">"%TempGalaxyFile%"

@echo delete all tables for restoration ...
REM DROP ALL TABLES FOR RESTORE CLEAN DATABASE
SET DropTables=drop table
SET Sep=
FOR /f "skip=1 delims=" %%a IN ('type "%TempGalaxyFile%"') DO SET DropTables=!DropTables!!Sep! %%a & SET Set=,
mysql %MySqlUser% --execute="unlock tables"
mysql %MySqlUser%" --execute="%DropTables%" 2>"%TempGalaxyFile%"
IF EXIST "%TempGalaxyFile%" SET /p a=<"%TempGalaxyFile%"
IF DEFINED a 					SET ErrorMessage= mysql cannot drop tables: %a%	& goto Error
del /q "%TempGalaxyFile%" 2>NUL

@echo Restore GexDb with last SqlDump and all bin-logs ...
FOR /f %%a IN ('dir /B "%BackupDir%%LastBackupDir%\*.sql"') DO mysql %MySqlUser% < "%GexBackupDir%%LastBackupDir%\%%a" &  del "%GexBackupDir%%LastBackupDir%\%%a"
FOR /f %%a IN ('dir /B "%BackupDir%%LastBackupDir%\*.0?????" /O') DO mysqlbinlog "%GexBackupDir%%LastBackupDir%\%%a" | mysql %MySqlUser% &  del "%GexBackupDir%%LastBackupDir%\%%a"

@echo Restart the server ...
net stop mysql >NUL
del /q "%datadir%%log-bin%.0?????" 2>NUL
del /q "%datadir%%log-bin%.index" 2>NUL
REM restart seveur
net start mysql
@echo Restoration OK
goto Exit

:Error
EndLocal
SET CallErrorMessage=%ErrorMessage%
IF NoDisplay goto Exit
IF DEFINED CallErrorMessage @echo ERROR:%CallErrorMessage%


:Exit
IF DEFINED TempGalaxyFile del /q "%TempGalaxyFile%" 2>NUL
EndLocal
IF NOT DEFINED NoDisplay Pause

