@echo OFF
SET CallErrorMessage=

REM SetLocal

SET ProgramName=gexdb_backup_incremental
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

REM this program compress and save a backup of all bin-log (- the last : can be used by the server)

SET TempGalaxyFile=%ProgramName%TempGalaxyFile.tmp
IF DEFINED TempGalaxyFile del /q "%TempGalaxyFile%" 2>NUL

@echo Read MySql and gexdb Configuration File ...
CALL gexdb_readconfig.bat %EchoOn% -myini -gexdbini
IF DEFINED CallErrorMessage			SET ErrorMessage=%CallErrorMessage% & goto Error

IF NOT DEFINED MySqlIni 			SET ErrorMessage= cannot find my.ini in %CD%\..\ & goto Error
IF NOT DEFINED datadir 				SET ErrorMessage= no datadir defined in my.ini & goto Error
IF NOT DEFINED GexDbIni 			SET ErrorMessage= cannot find gexdb.ini in %CD% & goto Error
IF NOT DEFINED GexAdminName 			SET ErrorMessage= GexDb Administrator not defined 	& goto Error
IF NOT DEFINED GexUserName 			SET ErrorMessage= GexDb User not defined 		& goto Error
IF NOT DEFINED GexReplicationName 		SET ErrorMessage= GexDb Replication not defined 	& goto Error
IF NOT DEFINED GexBackupDir 			SET ErrorMessage= GexBackupDir not defined	& goto Error

IF NOT EXIST "%GexBackupDir%" 			SET ErrorMessage= BackupDir %GexBackupDir% doesn't exist & goto Error

IF NOT DEFINED log-bin SET log-bin=*-bin

REM SEARCH THE LAST BACKUP DIR USED FOR STORED DUMP AND BINLOG FILES
REM BACKUPDIR IS LIKE 20061224
SET LastBackupDir=
FOR /f %%a IN ('dir /B /A:D "%GexBackupDir%20*" /O') DO SET LastBackupDir=%%a
IF NOT DEFINED LastBackupDir 		SET ErrorMessage= no LastBackupDir & goto Error

@echo Zip all bin-log et move it in the BackupDir ...
REM ZIP ALL BINLOGS AND COPY TO BACKUPDIR
REM DELETE OLD BINLOGS BUT KEEP THE LAST
REM RESTART THE SERVER TO CREATE NEW BINLOG

REM LAST BINLOGS
FOR /f %%a IN ('dir /B "%datadir%%log-bin%.0?????" /O ^| find /v "relay"') DO SET LastBinLogs=%%a
CALL GexDbGetSlaveStatus
IF NOT DEFINED GexBinLog	SET GexBinLog=%LastBinLog%
IF LastBinLog LSS GextBinLog 	SET GexBinLog=%LastBinLog%

:LabelSaveBinLog

REM SAVE BINLOG IN THE LAST BACKUP
REM stop serveur for zip backup
net stop mysql >NUL
FOR /f %%a IN ('dir /B "%datadir%%log-bin%.0?????" /O ^| find /v "relay"') DO IF "%%a"=="%LastBinLogs%"  (goto LabelCopy)  ELSE (zip -D "%datadir%%%a.zip" "%datadir%%%a" >NUL & del /q "%datadir%%%a" 2>NUL)
:LabelCopy
copy /Y "%datadir%%log-bin%.0?????.zip" "%GexBackupDir%%LastBackupDir%" 2>NUL >NUL
del /q "%datadir%%log-bin%.0?????.zip"	2>NUL

REM restart seveur
net start mysql 2>"%TempGalaxyFile%"
IF EXIST "%TempGalaxyFile%" SET /p a=<"%TempGalaxyFile%"
IF DEFINED a SET ErrorMessage= mysql server : %a%	& goto Error

del /q "%TempGalaxyFile%" 2>NUL
@echo Incremental Backup OK
goto Exit

:Error
SET CallErrorMessage=%ErrorMessage%
REM EndLocal
IF DEFINED NoDisplay goto Exit
IF DEFINED CallErrorMessage @echo ERROR:%CallErrorMessage%

:Exit
IF DEFINED TempGalaxyFile del /q "%TempGalaxyFile%" 2>NUL
REM EndLocal
IF NOT DEFINED NoDisplay Pause

