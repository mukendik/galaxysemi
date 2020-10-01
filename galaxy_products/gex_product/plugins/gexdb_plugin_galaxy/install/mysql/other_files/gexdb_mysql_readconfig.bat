@echo OFF
SET CallErrorMessage=
SET EchoOn=
SET ReadMySqlIni=
SET ReadGexDbIni=

REM ============================================================
REM GEXDB_READCONFIG OPTIONS
REM ============================================================

:AddCallParams
IF {%1}=={}                 goto FinAddCallParams
IF {%1}=={-echo}            SET EchoOn=%1 & SHIFT & goto AddCallParams
IF {%1}=={-myini}           SET ReadMySqlIni=1 & SHIFT & goto AddCallParams
IF {%1}=={-gexdbini}        SET ReadGexDbIni=1 & SHIFT & goto AddCallParams
@echo Usage :
@echo Usage : -echo         : display all lines
@echo Usage : -myini        : read my.ini
@echo Usage : -gexdbini     : read gexdb.ini
goto :eof
:FinAddCallParams

IF DEFINED EchoON @echo ON

REM ============================================================
REM this program only read the MySql and GexDb Config Files
REM ============================================================

IF NOT DEFINED ReadMySqlIni goto EndReadMySqlIni

REM ============================================================
REM the PWD (Program Word Directory) must be the "bin" dir of the MySql Server
REM ============================================================
SET MySqlDir=..\

REM ============================================================
REM RESET VARIABLES
SET server-id=
SET log-bin=
SET log-slave-updates=
SET slave_compressed_protocol=
SET binlog-do-db=
SET replicate-same-server-id=
SET report-host=
SET master-host=
SET master-port=
SET master-user=
SET master-password=
SET replicate-do-db=
SET datadir=
SET innodb_file_per_table=


REM ============================================================
REM read the mysql config file : my.ini to find the datadir
REM read the gexdb config file : gexdb.ini
REM ============================================================

SET MySqlIni=%MySqlDir%my.ini
IF NOT EXIST "%MySqlIni%"              SET MySqlIni=
IF NOT DEFINED MySqlIni                SET ErrorMessage= cannot find my.ini in %CD%\..\ & goto Error

REM ============================================================
REM SEARCH THE VALUE OF DATADIR IN MY.INI
REM DATADIR IS THE DIRECTORY WHERE THE DATABASE WAS STORED
REM for all var def with value
for /f "delims=" %%a in ('type "%MySqlIni%"^|find "="^|find /v "#"') do SET %%a
REM for all other var def without value
for /f "delims=" %%a in ('type "%MySqlIni%"^|find /v"="^|find /v"["^|find /v "#"') do SET %%a=1

IF NOT DEFINED datadir                 SET ErrorMessage= no datadir defined in my.ini & goto Error

REM ============================================================
REM HAVE FOUND A VALUE FOR DATADIR : FORMAT IT
REM REPLACE " wITH §
SET datadir=%datadir:"=§%
CALL :SubRoutineFormatPath "%datadir%"
SET datadir=%Result:§=%

:EndReadMySqlIni

REM ============================================================
REM READ GEXDBINI CONFIGURATION FILE
REM ============================================================

IF NOT DEFINED ReadGexDbIni goto EndReadGexDbIni
IF NOT DEFINED datadir goto :eof

SET GexServerStatus=master
SET GexServerId=1
SET GexDataBaseName=
SET GexAdminName=
SET GexAdminPwd=
SET GexUserName=
SET GexUserPwd=
SET GexReplicationName=
SET GexReplicationPwd=
SET GexBinLogName=
SET GexMasterHost=
SET GexSlaveHost=
SET GexBackupDir=
SET GexPurgeSplitlotSamplesAfterNbDays=
SET GexPurgeSplitlotStatsAfterNbDays=
SET GexPurgeSpitlotEntryAfterNbDays=
SET GexPurgeHour=
SET GexPurgeEvery=
SET GexBackupIncrementalHour=
SET GexBackupIncrementalEvery=
SET GexBackupTotalHour=
SET GexBackupTotalEvery=
SET GexCheckAndRepairHour=
SET GexCheckAndRepairEvery=


SET GexDbIni=gexdb.ini
IF NOT EXIST "%GexDbIni%"         SET GexDbIni=
IF NOT DEFINED GexDbIni           SET ErrorMessage= cannot find gexdb.ini in %datadir% & goto Error

REM ============================================================
REM SEARCH THE GEXDB CONFIGURATION GEXDB.INI STORED IN DATADIR WITH THE GEXDB DATABASE
REM ALL DATA IN THIS FILE HAVE TO BE SET
for /f "delims=" %%a in ('type "%GexDbIni%"^|find "="^|find /v "#"^|find /v "<"^|find /v ">"') do SET %%a

IF NOT DEFINED GexBackupDir        goto EndReadGexDbIni
REM ============================================================
REM HAVE FOUND A VALUE FOR DATADIR GEXBACKUPDIR : FORMAT IT
REM REPLACE " WITH §
SET GexBackupDir=%GexBackupDir:"=§%
CALL :SubRoutineFormatPath "%GexBackupDir%"
SET GexBackupDir=%Result:§=%


:EndReadGexDbIni

REM ============================================================
REM FORMAT DATADIR
REM ============================================================

SET DateDir=%date:/=%
for /f %%a in ('echo ^|date ^| find "jj-mm"') do SET DateDir=%date:~-4%%date:~-7,2%%date:~-10,2%& goto EndReadConfig
for /f %%a in ('echo ^|date ^| find "mm-jj"') do SET DateDir=%date:~-4%%date:~-10,2%%date:~-7,2%& goto EndReadConfig
for /f %%a in ('echo ^|date ^| find "dd-mm"') do SET DateDir=%date:~-4%%date:~-7,2%%date:~-10,2%& goto EndReadConfig
for /f %%a in ('echo ^|date ^| find "mm-dd"') do SET DateDir=%date:~-4%%date:~-10,2%%date:~-7,2%& goto EndReadConfig


:EndReadConfig

goto :Exit

REM ============================================================
REM SUBROUTINE SECTION
REM ============================================================

:SubRoutineFormatPath
SET Result=%1
SET Result=%Result:§=%
SET Result=%Result:"=%
REM REMOVE SPACE AT END
:RepeatSubRoutineFormatPath
IF NOT "%Result:~-1%"==" " goto :StopSubRoutineFormatPath
SET Result=%Result:~0,-1%
goto :RepeatSubRoutineFormatPath
:StopSubRoutineFormatPath
SET Result=%Result:/=\%
SET Result=%Result%\
SET Result=%Result:\\=\%
goto :eof

goto Exit

REM ============================================================
REM EXIT SECTION
REM ============================================================

:Error
SET CallErrorMessage=%ErrorMessage%

:Exit
