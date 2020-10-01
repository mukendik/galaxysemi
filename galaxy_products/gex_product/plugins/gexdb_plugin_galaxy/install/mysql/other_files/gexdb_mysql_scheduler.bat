@echo OFF

SET CallErrorMessage=

SetLocal
SET ProgramName=%0
SET TempGalaxyFile=%ProgramName%TempGalaxyFile.tmp
SET EchoOn=
SET Install=
SET PauseMode=
SET NoDisplay=
SET NoReadConfig=
SET a=

REM ============================================================
REM GEXDB_SCHEDULER OPTIONS
REM ============================================================

:AddCallParams
IF {%1}=={}                    goto FinAddCallParams
IF {%1}=={-echo}               SET EchoOn=%1 & SHIFT & goto AddCallParams
IF {%1}=={-pause}              SET PauseMode=%1 & SHIFT & goto AddCallParams
IF {%1}=={-nodisplay}          SET NoDisplay=%1 & SHIFT & goto AddCallParams
IF {%1}=={-install}            SET Install=%1 & SHIFT & goto AddCallParams
IF {%1}=={-noreadconfig}       SET NoReadConfig=%1 & SHIFT & goto AddCallParams
@echo Usage :
@echo Usage : -echo            : display all lines
@echo Usage : -pause           : add some pause
@echo Usage : -nodisplay       : for monitoring
@echo Usage : -install         : for backupdir creation
@echo Usage : -noreadconfig    : when config file already loaded

goto Exit

:FinAddCallParams
IF DEFINED NoDisplay SET EchoOn=
IF DEFINED NoDisplay SET PauseMode=

IF DEFINED EchoOn @echo ON
REM ============================================================
REM this program add tasks for purge, backup and cleanup in scheduler tasks manager
REM ============================================================

IF DEFINED NoReadConfig                goto EndReadConfig
@echo Read MySql and GexDb Configuration file ...
call gexdb_mysql_readconfig.bat %EchoOn% -myini -gexdbini

:EndReadConfig

IF NOT DEFINED MySqlIni                         SET ErrorMessage= cannot find my.ini in %CD%\..\ & goto Error
IF NOT DEFINED datadir                          SET ErrorMessage= no datadir defined in my.ini & goto Error
IF NOT DEFINED GexDbIni                         SET ErrorMessage= cannot find gexdb.ini in %CD% & goto Error
REM IF NOT DEFINED GexBackupDir                 SET ErrorMessage= GexBackupDir not defined & goto Error

IF DEFINED TempGalaxyFile del /q "%TempGalaxyFile%" 2>NUL

REM ============================================================
REM ADD TASKS IN THE WINDOWS SCHEDULER MANAGER
REM ============================================================

SET HaveToAddNewTask=
IF NOT "%GexServerStatus%"=="master"            goto AddTasks
IF NOT DEFINED GexPurgeEvery                    goto AddBackupInc
REM ============================================================
REM Add Purge if master
   @echo *** Purge information : del splitlot samples after %GexPurgeSplitlotSamplesAfterNbDays% days
   @echo *** Purge information : del splitlot stats after %GexPurgeSplitlotStatsAfterNbDays% days
   @echo *** Purge information : del splitlot entries after %GexPurgeSpitlotEntryAfterNbDays% days

   @echo *** DO YOU WANT TO ADD PURGE TASK IN WINDOWS SCHEDULER MANAGER TO ***
SET a=
SET /p a=*** PURGE THE GEXDB DATABASE ON THE MASTER SERVER ?   (Y for yes) ***: 
IF NOT DEFINED a                                SET a=n
IF NOT "%a%"=="Y" IF NOT "%a%"=="y"             SET GexPurgeEvery=
IF DEFINED GexPurgeEvery                        SET HaveToAddNewTask=1

:AddBackupInc
IF NOT DEFINED GexBackupIncrementalEvery        goto AddBackupTotal
REM ============================================================
REM Add Backup on this server
   @echo *** DO YOU WANT TO ADD PURGE TASK IN WINDOWS SCHEDULER MANAGER TO ***
SET a=
SET /p a=*** MAKE INCREMENTAL BACKUP ON THIS SERVER ?          (Y for yes) ***: 
IF NOT DEFINED a                                SET a=n
IF NOT "%a%"=="Y" IF NOT "%a%"=="y"             SET GexBackupIncrementalEvery=
IF DEFINED GexBackupIncrementalEvery            SET HaveToAddNewTask=1

:AddBackupTotal
IF NOT DEFINED GexBackupTotalEvery              goto AddTasks
REM ============================================================
REM Add Backup on this server
   @echo *** DO YOU WANT TO ADD PURGE TASK IN WINDOWS SCHEDULER MANAGER TO ***
SET a=
SET /p a=*** MAKE TOTAL BACKUP ON THIS SERVER ?                (Y for yes) ***: 
IF NOT DEFINED a                                SET a=n
IF NOT "%a%"=="Y" IF NOT "%a%"=="y"             SET GexBackupTotalEvery=
IF DEFINED GexBackupTotalEvery                  SET HaveToAddNewTask=1

:AddTasks
IF DEFINED GexCheckAndRepairEvery               SET HaveToAddNewTask=1

IF NOT DEFINED HaveToAddNewTask goto LabelDelTask
@echo Add tasks in Windows Sheduler ...
REM ============================================================
REM FRENCH OR ENGLISH COMMAND
for /f "delims=" %%a in ('at /? ^| find /I "jeudi"') do SET FrenchDay=1

IF NOT DEFINED FrenchDay goto LabelDelTask

IF DEFINED GexPurgeEvery (CALL :SubRoutineFormatFrenchDay %GexPurgeEvery%)
IF DEFINED GexPurgeEvery SET GexPurgeEvery=%Result%

IF DEFINED GexBackupIncrementalEvery (CALL :SubRoutineFormatFrenchDay %GexBackupIncrementalEvery%)
IF DEFINED GexBackupIncrementalEvery SET GexBackupIncrementalEvery=%Result%

IF DEFINED GexBackupTotalEvery (CALL :SubRoutineFormatFrenchDay %GexBackupTotalEvery%)
IF DEFINED GexBackupTotalEvery SET GexBackupTotalEvery=%Result%

IF DEFINED GexCheckAndRepairEvery (CALL :SubRoutineFormatFrenchDay %GexCheckAndRepairEvery%)
IF DEFINED GexCheckAndRepairEvery SET GexCheckAndRepairEvery=%Result%

:LabelDelTask
REM ============================================================
REM DELETE OLD TASK
for /f "tokens=1 delims= " %%a in ('AT ^| find "gexdb_mysql_"') do AT %%a /DELETE 2>NUL >NUL

IF NOT DEFINED HaveToAddNewTask goto LabelEndScheduler
REM ============================================================
REM ADD PURGE TASK
:LabelPurge
SET a=
IF NOT DEFINED GexPurgeEvery goto LabelInc
IF NOT DEFINED GexPurgeHour SET GexPurgeHour=00:00
AT  %GexPurgeHour% /EVERY:%GexPurgeEvery% "%CD%\gexdb_mysql_purge.bat" -nodisplay 2>"%TempGalaxyFile%" >NUL
IF EXIST "%TempGalaxyFile%" SET /p a=<"%TempGalaxyFile%"
IF DEFINED a SET ErrorMessage= cannot add task GexPurge & goto Error

REM ============================================================
REM ADD INCREMENTAL BACKUP TASK
:LabelInc
SET a=
IF NOT DEFINED GexBackupIncrementalEvery goto LabelTotal
IF NOT DEFINED GexBackupIncrementalHour SET GexBackupIncrementalHour=00:00
AT  %GexBackupIncrementalHour% /EVERY:%GexBackupIncrementalEvery% "%CD%\gexdb_mysql_backup_incremental.bat" -nodisplay 2>"%TempGalaxyFile%" >NUL
IF EXIST "%TempGalaxyFile%" SET /p a=<"%TempGalaxyFile%"
IF DEFINED a SET ErrorMessage= cannot add task GexBackupIncrementalId & goto Error

REM ============================================================
REM ADD TOTAL BACKUP TASK
:LabelTotal
SET a=
IF NOT DEFINED GexBackupTotalEvery goto LabelCheckAndRepair
IF NOT DEFINED GexBackupTotalHour SET GexBackupTotalHour=00:00
AT  %GexBackupTotalHour% /EVERY:%GexBackupTotalEvery% "%CD%\gexdb_mysql_backup_total.bat" -nodisplay 2>"%TempGalaxyFile%" >NUL
IF EXIST "%TempGalaxyFile%" SET /p a=<"%TempGalaxyFile%"
IF DEFINED a SET ErrorMessage= cannot add task GexBackupTotalId & goto Error

REM ============================================================
REM ADD CHECK AND REPAIR TASK
:LabelCheckAndRepair
SET a=
IF NOT DEFINED GexCheckAndRepairEvery goto LabelSaveIni
IF NOT DEFINED GexCheckAndRepairHour SET GexCheckAndRepairHour=00:00
AT  %GexCheckAndRepairHour% /EVERY:%GexCheckAndRepairEvery% "%CD%\gexdb_mysql_check_and_repair.bat" -nodisplay 2>"%TempGalaxyFile%" >NUL
IF EXIST "%TempGalaxyFile%" SET /p a=<"%TempGalaxyFile%"
IF DEFINED a SET ErrorMessage= cannot add task GexCheckAndRepairId & goto Error

REM ============================================================
REM SAVE GEXDB CONFIGURATION FILE
:LabelSaveIni
@echo Update GexDb configuration file ...
REM SAVE NEW TASK ID IN GEXDB.INI
IF DEFINED TempGalaxyFile del /q "%TempGalaxyFile%" 2>NUL
(@echo # GEXDB LAST SAVED %date%)>"%TempGalaxyFile%"

SET HaveComment=1
for /f "delims=" %%a in ('type "%GexDbIni%"^|find /v "<"^|find /v ">"') do SET Params=%%a & CALL :SubRoutineWriteFile

del /q "%GexDbIni%" 2>NUL
copy /y "%TempGalaxyFile%" "%GexDbIni%" >NUL
del /q "%TempGalaxyFile%" 2>NUL

IF NOT DEFINED GexBackupDir                     goto LabelEndScheduler
IF DEFINED Install IF NOT EXIST "%GexBackupDir%%DateDir%" @echo Create the BackupDir "%GexBackupDir%%DateDir%"
IF DEFINED Install IF NOT EXIST "%GexBackupDir%%DateDir%" mkdir "%GexBackupDir%%DateDir%" 2>NUL >NUL

REM ============================================================
:LabelEndScheduler
@echo GexDbSheduler OK
goto Exit

REM ============================================================
REM SUBROUTINE SECTION
REM ============================================================

:SubRoutineWriteFile
SET MyId=%Params%
SET MyId=%MyId:"=§%

:SupprEndSpace
IF NOT "%MyId:~-1%"==" " goto FinSupprEndSpace
SET MyId=%MyId:~0,-1%
goto SupprEndSpace
:FinSupprEndSpace


IF "%MyId:~2,17%"=="GEXDB LAST SAVED" goto :eof
REM INSERT LINE BEFORE [chapter]
REM INSERT LINE BEFORE # comment
IF "%MyId:~0,1%"=="["                           (@echo\)>>%TempGalaxyFile% & (@echo\)>>"%TempGalaxyFile%
IF "%MyId:~0,1%"=="#" IF NOT DEFINED HaveComment SET HaveComment=1 & (@echo\)>>%TempGalaxyFile%
IF NOT "%MyId:~0,1%"=="#" SET HaveComment=

IF "%MyId:~0,1%"=="#"                           @echo %MyId:§="% >>%TempGalaxyFile% & goto :eof
IF "%MyId:~0,1%"=="["                           (@echo %MyId:§="%)>>%TempGalaxyFile% & goto :eof

IF NOT DEFINED GexPurgeEvery                    IF "%MyId:~0,8%"=="GexPurge"                 (@echo #%MyId:§="%)>>%TempGalaxyFile% & goto :eof
IF NOT DEFINED GexBackupTotalEvery              IF "%MyId:~0,14%"=="GexBackupTotal"         (@echo #%MyId:§="%)>>%TempGalaxyFile% & goto :eof
IF NOT DEFINED GexBackupIncrementalEvery        IF "%MyId:~0,20%"=="GexBackupIncremental" (@echo #%MyId:§="%)>>%TempGalaxyFile% & goto :eof

(@echo %MyId:§="%)>>%TempGalaxyFile%
goto :eof

:SubRoutineFormatFrenchDay
SET Result=%1
IF {%1}=={Su} SET Result=D  & goto :eof
IF {%1}=={F}  SET Result=V  & goto :eof
IF {%1}=={Th} SET Result=J  & goto :eof
IF {%1}=={M}  SET Result=L  & goto :eof
IF {%1}=={W}  SET Result=Me & goto :eof
IF {%1}=={T}  SET Result=Ma & goto :eof

goto :eof

REM ============================================================
REM EXIT SECTION
REM ============================================================

:Error
EndLocal
SET CallErrorMessage=%ErrorMessage%
IF NoDisplay goto Exit
IF DEFINED CallErrorMessage @echo ERROR:%CallErrorMessage%

:Exit

IF DEFINED TempGalaxyFile del /q "%TempGalaxyFile%" 2>NUL
IF NOT DEFINED NoDisplay Pause
EndLocal