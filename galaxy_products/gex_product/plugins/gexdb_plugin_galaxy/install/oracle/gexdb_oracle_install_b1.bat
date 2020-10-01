@echo OFF
SET CallErrorMessage=
cls
SET ProgramName=%0
SET EchoOn=
SET PauseMode=
SET a=
SET TempGalaxyFile=gexdb_oracle_installTempGalaxyFile.tmp

REM ============================================================
REM GEXDB_ORACLE_INSTALL OPTIONS
REM ============================================================

:AddCallParams
IF {%1}=={} 			goto FinAddCallParams
IF {%1}=={-echo} 		SET EchoOn=%1 & SHIFT & goto AddCallParams
IF {%1}=={-pause} 		SET PauseMode=%1 & SHIFT & goto AddCallParams
IF {%1}=={-nodisplay} 	SET NoDisplay=%1 & SHIFT & goto AddCallParams
@echo Usage :
@echo Usage : -echo 		: display all lines
@echo Usage : -pause 		: script executes some pause instructions
@echo Usage : -nodisplay 	: for monitoring
goto Exit

:FinAddCallParams
IF DEFINED NoDisplay SET EchoOn= & SET PauseMode=

IF DEFINED EchoON @echo ON

REM ============================================================
REM READ GEXDB CONFIGURATION
REM USE SQL SCRIPT TO CREATE GEXDB USERS, TABLES, PROCEDURES
REM ASK FOR SCHEDULER TASK (HOT BACKUP, TOTAL BACKUP, INCR BACKUP, PURGE)
REM ============================================================

SET GexDbIni=gexdb_oracle_install.ini
SET GexDbInstallSql=gexdb_oracle_install_b1.sql
SET GexDbUninstallSql=gexdb_oracle_uninstall.sql

IF NOT EXIST "%GexDbIni%"				SET ErrorMessage= Cannot find file %GexDbIni% in %CD% & goto Error
IF NOT EXIST "%GexDbInstallSql%"		SET ErrorMessage= Cannot find file %GexDbSql% in %CD% & goto Error
IF NOT EXIST "%GexDbUninstallSql%"		SET GexDbUninstallSql=

@echo.
@echo ***************************************
@echo *                                     *
@echo *      Galaxy GexDB SQL database      *
@echo *                                     *
@echo ***************************************
@echo.

IF DEFINED PauseMode PAUSE

REM ============================================================
REM READ GEXDB CONFIGURATION FILE
REM ============================================================

echo o Reading configuration file %GexDbIni%...

call gexdb_oracle_readconfig.bat %EchoOn% -gexdbini
IF DEFINED CallErrorMessage			SET ErrorMessage=%CallErrorMessage%			& goto Error

IF DEFINED PauseMode PAUSE

IF NOT DEFINED GexDbIni 				SET ErrorMessage= Cannot find gexdb.ini in %datadir% 	& goto Error
IF NOT DEFINED GexDataBaseInstance		SET ErrorMessage= Oracle instance not defined 	& goto Error
IF NOT DEFINED GexAdminName 			SET ErrorMessage= GexDb Administrator not defined 	& goto Error
IF NOT DEFINED GexUserName 				SET ErrorMessage= GexDb User not defined 		& goto Error

IF NOT DEFINED GexDataBaseLoggingMode	SET GexDataBaseLoggingMode=NOLOGGING

:DataBaseCreation

IF DEFINED PauseMode PAUSE

@echo.
@echo o Please enter SYSDBA login details:
IF DEFINED TempGalaxyFile del /q "%TempGalaxyFile%" 2>NUL

REM ============================================================
REM CREATE THE NEW GEXDB
REM ASK FOR ROOT NAME AND ROOT PWD FOR GEXDB CREATION
REM ============================================================

SET NbTry=0
:AskForRoot
SET RootName=
SET RootPwd=
SET DbInstance=
SET a=
SET /a NbTry=%NbTry%+1
SET /p RootName=  User     : 
SET /p  RootPwd=  Password : 
SET OracleRoot=%RootName%/%RootPwd%@%GexDataBaseInstance% AS SYSDBA
REM Test connection
echo exit;>%TempGalaxyFile%.sql
(sqlplus -L %OracleRoot% @%TempGalaxyFile%.sql) >"%TempGalaxyFile%"
IF EXIST "%TempGalaxyFile%.sql" 	del /q "%TempGalaxyFile%.sql" 2>NUL
IF EXIST "%TempGalaxyFile%"		for /f "delims=" %%a in ('type "%TempGalaxyFile%"^|find "ORA-"') do SET a=%%a
IF DEFINED a @echo   ******* Oracle connection: cannot connect with %OracleRoot%
IF DEFINED a @echo   ******* %a%
IF DEFINED a IF 2 lss %NbTry%		SET ErrorMessage= cannot connect to Oracle Server	& goto Error
IF DEFINED TempGalaxyFile 		del /q "%TempGalaxyFile%" 2>NUL
IF DEFINED a goto AskForRoot
IF DEFINED PauseMode PAUSE

REM ============================================================
REM CREATE AN EMPTY GEXDB
REM Add the SqlStatement Exit; at the end of the script to exit of the sqlplus console
REM ============================================================

IF NOT DEFINED GexDbUninstallSql	goto LabelInstallSql
REM ============================================================
REM UNINSTALL GEXDB
REM ============================================================

@echo.
@echo o Uninstalling existing GexDb schema ...
SET a=
for /f "delims=" %%a in ('type "%GexDbUninstallSql%"^|find /I "exit;"') do SET a=%%a
IF NOT DEFINED a	echo exit;>>%GexDbUninstallSql%
(sqlplus -L %OracleRoot% @%GexDbUninstallSql% %GexAdminName%) >"%TempGalaxyFile%"
SET a=
for /f "delims=" %%a in ('type "%TempGalaxyFile%"^|find /V "ORA-06512"^|find /I "ORA-"') do SET a=%%a
IF DEFINED PauseMode PAUSE
IF DEFINED a SET ErrorMessage= GexDb Schema delete fail : %a%	& goto Error
IF DEFINED TempGalaxyFile 		del /q "%TempGalaxyFile%" 2>NUL

REM ============================================================
REM INSTALL GEXDB
REM ============================================================

:LabelInstallSql
@echo.
@echo o Creating GexDb schema ...
SET a=
for /f "delims=" %%a in ('type "%GexDbInstallSql%"^|find /I "exit;"') do SET a=%%a
IF NOT DEFINED a	echo exit;>>%GexDbInstallSql%
(sqlplus -L %OracleRoot% @%GexDbInstallSql% %GexAdminName% %GexDataBaseLoggingMode%) >"%TempGalaxyFile%"
SET a=
for /f "delims=" %%a in ('type "%TempGalaxyFile%"^|find /V "01543"^|find /V "01920"^|find /V "00955"^|find "ORA-"') do SET a=%%a
IF DEFINED PauseMode PAUSE
IF DEFINED a SET ErrorMessage= GexDb Schema creation fail : %a%	& goto Error
IF DEFINED TempGalaxyFile 		del /q "%TempGalaxyFile%" 2>NUL

REM ============================================================
REM ADD SCHEDULED TASKS IN ORACLE SCHEDULER
REM ============================================================

SET HaveScheduler=
IF DEFINED GexPurgeJobName				SET HaveScheduler=1
IF DEFINED GexBackupIncrementalEvery	SET HaveScheduler=1
IF DEFINED GexBackupTotalEvery			SET HaveScheduler=1
IF DEFINED GexHotBackupEvery			SET HaveScheduler=1

IF NOT DEFINED HaveScheduler			goto EndInstall

REM ============================================================
REM CREATE CALL OF ADD_PURGE_JOB STORED PROCEDURE
REM ============================================================
echo -- CALL %GexAdminName%."ADD_PURGE_JOB"()	>"%TempGalaxyFile%.sql"
echo CALL %GexAdminName%."ADD_PURGE_JOB"(		>>"%TempGalaxyFile%.sql"
echo		'%GexPurgeJobName%',				>>"%TempGalaxyFile%.sql"
echo		'%GexPurgeJobRepeatInterval%',		>"%TempGalaxyFile%.sql"
echo		'PURGE JOB CONFIGURATION',			>>"%TempGalaxyFile%.sql"
echo		'%GexPurgeSplitlotSamplesAfterNbWeeks%',>>"%TempGalaxyFile%.sql"
echo		'%GexPurgeSplitlotStatsAfterNbWeeks%',>>"%TempGalaxyFile%.sql"
echo		'%GexPurgeSplitlotEntriesAfterNbWeeks%'); >>"%TempGalaxyFile%.sql"
echo exit; >>"%TempGalaxyFile%.sql"

SET OracleUser=%GexAdminName%/%GexAdminPwd%@%GexDataBaseInstance%
IF DEFINED PauseMode Pause
(sqlplus %OracleUser% @"%TempGalaxyFile%.sql")>"%TempGalaxyFile%"
IF DEFINED PauseMode Pause

IF EXIST "%TempGalaxyFile%.sql" 	del /q "%TempGalaxyFile%.sql" 2>NUL
IF EXIST "%TempGalaxyFile%"			for /f "delims=" %%a in ('type "%TempGalaxyFile%"^|find "ORA-"') do SET a=%%a
IF DEFINED a SET ErrorMessage= mysql connection : %a%	& goto Error

@echo End of purge OK
REM CALL gexdb_oracle_scheduler.bat %EchoOn% -install -noreadconfig -nodisplay
REM IF DEFINED CallErrorMessage			SET ErrorMessage=%CallErrorMessage%	& goto Error


IF NOT DEFINED GexHotBackupDir		goto EndInstall
REM ============================================================
REM CREATE THE FIRST BACKUP
REM if GexBackup dir not defined then ERROR : can't access to the backup server
REM HOT BACKUP
REM ============================================================

@echo Backup the first HotBackup ...
IF DEFINED TempGalaxyFile del /q "%TempGalaxyFile%" 2>NUL

CALL %GexHotBackupBatch%

IF DEFINED PauseMode PAUSE
:EndInstall

REM ============================================================
REM END OF GEXDB INSTALLATION
REM ============================================================

@echo.
@echo **** Success: GexDb schema created
@echo.

goto Exit

REM ============================================================
REM EXIT SECTION
REM ============================================================

:Error
SET CallErrorMessage=%ErrorMessage%
IF DEFINED CallErrorMessage	@echo.
IF DEFINED CallErrorMessage	@echo ERROR:%CallErrorMessage%
IF DEFINED CallErrorMessage	@echo.
IF DEFINED PauseMode PAUSE

:Exit
IF NOT DEFINED NoDisplay Pause
IF DEFINED TempGalaxyFile del /q "%TempGalaxyFile%" 2>NUL
