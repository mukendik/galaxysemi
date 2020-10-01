@echo OFF
SET CallEroorMessage=

REM SetLocal
SET ProgramName=gexdb_backup_total
SET EchoOn=
SET PauseMode=
SET NoDisplay=
SET a=

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

REM this program compress and save a backup of all bin-log (- the last : can be used by the slave)

SET TempGalaxyFile=%ProgramName%TempGalaxyFile.tmp
del /q "%TempGalaxyFile%" 2>NUL

@echo Read MySql and GexDb Configuration File ...
call gexdb_readconfig.bat %EchoOn% -myini -gexdbini
IF DEFINED CallErrorMessage			SET ErrotMessage=%CallErrorMessage% & goto Error

IF NOT DEFINED MySqlIni 			SET ErrorMessage= cannot find my.ini in %CD%\..\ & goto Error
IF NOT DEFINED datadir 				SET ErrorMessage= no datadir defined in my.ini & goto Error
IF NOT DEFINED GexDbIni 			SET ErrorMessage= cannot find GexDb.ini in %datadir% & goto Error
IF NOT DEFINED GexAdminName 			SET ErrorMessage= GexDb Administrator not defined 	& goto Error
IF NOT DEFINED GexUserName 			SET ErrorMessage= GexDb User not defined 		& goto Error
IF NOT DEFINED GexReplicationName 		SET ErrorMessage= GexDb Replication not defined 	& goto Error
IF NOT DEFINED GexBackupDir 			SET ErrorMessage= GexBackupDir not defined	& goto Error

IF NOT EXIST "%GexBackupDir%" @echo Create the GexDb Backup Dir &mkdir "%GexBackupDir%" 2>"%TempGalaxyFile%"
IF EXIST "%TempGalaxyFile%" SET /p a=<"%TempGalaxyFile%"
IF DEFINED a SET ErrorMessage= BackupDir Creation fail : %a%	& goto Error


IF NOT DEFINED log-bin SET log-bin=*-bin

REM SEARCH THE LAST BACKUP DIR USED FOR STORED DUMP AND BINLOG FILES
REM GEXBACKUPDIR IS LIKE 20061224
SET LastBackupDir=
FOR /f %%a IN ('dir /B /A:D "%GexBackupDir%20*" /O') DO SET LastBackupDir=%%a

REM DUMP ALL GEXDB
REM create the DateDir

REM echo %DateDir%
IF NOT DEFINED DateDir SET ErrorMessage= DateDir not found & goto Error
:CreateDateDir
IF NOT EXIST "%GexBackupDir%%DateDir%" @echo Create the GexDb Backup Dir %GexBackupDir%%DateDir% &mkdir "%GexBackupDir%%DateDir%" 2>"%TempGalaxyFile%"
IF EXIST "%TempGalaxyFile%" SET /p a=<"%TempGalaxyFile%"
IF DEFINED a SET ErrorMessage= BackupDir Creation fail : %a%	& goto Error

IF "%DateDir%"=="%LastBackupDir%" SET LastBackupDir= &del /q "%GexBackupDir%%DateDir%\*.zip" 2>NUL

REM SAVE BINLOG IN THE LAST BACKUP
REM LOCK ALL TABLES FROM THE GEXDB DATABASE
SET MySqlUser=--user=%GexAdminName% --password=%GexAdminPwd% %GexDataBaseName%
mysql %MySqlUser% --execute="show databases" 2>"%TempGalaxyFile%" >NUL
IF EXIST "%TempGalaxyFile%" SET /p a=<"%TempGalaxyFile%"
IF DEFINED a SET ErrorMessage= GexDb_admin connection fail: %a%	& goto Error

mysql %MySqlUser% --execute="show tables">"%TempGalaxyFile%"

@echo Lock all tables for dump ...
SET LockTables=lock tables
SET Sep=
FOR /f "skip=1 delims=" %%a IN ('type "%TempGalaxyFile%"') DO (SET LockTables=!LockTables!!Sep! %%a write) & SET Sep=,
del /q "%TempGalaxyFile%" 2>NUL

mysql %MySqlUser% --execute="%LockTables%" 2>"%TempGalaxyFile%"
IF EXIST "%TempGalaxyFile%" SET /p a=<"%TempGalaxyFile%"
IF DEFINED a SET ErrorMessage= cannot lock tables : %a%	& goto Error
REMFOR /f "skip=1 delims=" %%a IN ('type "%TempGalaxyFile%"') DO mysql %MySqlUser% --execute="%LockTables% %%a write"
del /q "%TempGalaxyFile%" 2>NUL

@echo Dump GexDb ...
REM DUMP THE GEXDB DATABASE IN A SQL FILE
mysqldump %MySqlUser% > "%datadir%%GexDataBaseName%_%DateDir%.sql"

@echo Restart the server ...
REM stop serveur for zip backup
net stop mysql >NUL
IF NOT DEFINED LastBackupDir GOTO TotalBackup

REM ZIP AND BACKUP THE LAST BINLOGS
FOR /f %%a IN ('dir /B "%datadir%%log-bin%.0?????" /O ^| find /v "relay"') DO zip -D "%datadir%%%a.zip" "%datadir%%%a" >NUL
IF DEFINED LastBackupDir copy /Y "%datadir%%log-bin%.0?????.zip" "%GexBackupDir%%LastBackupDir%" >NUL
del /q "%datadir%%log-bin%.0?????.zip" 2>NUL

REM CLEAN THE SERVER AND ZIP AND BACKUP THE SQL DUMP
:TotalBackup
del /q "%datadir%%log-bin%.00*" 2>NUL
del /q "%datadir%%log-bin%.index" 2>NUL

SET LastBinLog=''
zip -D "%datadir%%GexDataBaseName%_%DateDir%.sql.zip" "%datadir%%GexDataBaseName%_%DateDir%.sql" >NUL
copy /Y "%datadir%%GexDataBaseName%_%DateDir%.sql.zip" "%GexBackupDir%%DateDir%" >NUL
del /q "%datadir%%GexDataBaseName%_%DateDir%.sql.zip" 2>NUL
del /q "%datadir%%GexDataBaseName%_%DateDir%.sql" 2>NUL

REM restart seveur
net start mysql

REM VERIFY ALL TABLES ARE UNLOCKED
mysql %MySqlUser% --execute="unlock tables"

@echo GexDb Total Backup OK
goto Exit

:Error
REM EndLocal
SET CallErrorMessage=%ErrorMessage%
IF DEFINED NoDisplay goto Exit
IF DEFINED CallErrorMessage @echo ERROR:%CallErrorMessage%

:Exit
IF DEFINED TempGalaxyFile del /q "%TempGalaxyFile%" 2>NUL
REM EndLocal
IF NOT DEFINED NoDisplay Pause
