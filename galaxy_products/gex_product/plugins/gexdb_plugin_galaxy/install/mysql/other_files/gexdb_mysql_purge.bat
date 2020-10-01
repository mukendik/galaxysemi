@echo OFF
SET CallErrorMessage=

REM SetLocal
SET ProgramName=gexdb_purge
SET EchoOn=
SET PauseMode=
SET NoDisplay=
SET a=
SET TempGalaxyFile=

REM ============================================================
REM GEXDB_PURGE OPTIONS
REM ============================================================

:AddCallParams
IF {%1}=={}               goto FinAddCallParams
IF {%1}=={-echo}          SET EchoOn=%1 & SHIFT & goto AddCallParams
IF {%1}=={-pause}         SET PauseMode=%1 & SHIFT & goto AddCallParams
IF {%1}=={-nodisplay}     SET NoDisplay=%1 & SHIFT & goto AddCallParams
@echo Usage :
@echo Usage : -echo       : display all lines
@echo Usage : -pause      : add some pause
@echo Usage : -nodisplay  : for monitoring
goto Exit

:FinAddCallParams
IF DEFINED NoDisplay      SET EchoOn= & SET PauseMode=

REM ============================================================
REM this program call the MySql PURGE_SPLITLOTS to purge and cleanup GexDb database
REM ============================================================

SET TempGalaxyFile=%ProgramName%TempGalaxyFile.tmp
del /q "%TempGalaxyFile%" 2>NUL

REM ============================================================
REM READ MYSQL AND GEXDB CONFIGURATION FILE
REM ============================================================

@echo Read MySql and GexDb configuration file ...
call gexdb_mysql_readconfig.bat %EchoOn%  -myini -gexdbini
IF DEFINED CallErrorMessage                        SET ErrorMessage=%CallErrorMessage%                & goto Error

IF NOT DEFINED MySqlIni                            SET ErrorMessage= cannot find my.ini in %CD%\..\ & goto Error
IF NOT DEFINED datadir                             SET ErrorMessage= no datadir defined in my.ini & goto Error
IF NOT DEFINED GexDbIni                            SET ErrorMessage= cannot find gexdb.ini in %datadir% & goto Error

REM if GexDb Backup dir not defined then ERROR : can't access to the backup server
IF NOT DEFINED GexAdminName                        SET ErrorMessage= cannot find GexAdminName & goto Error
IF NOT DEFINED GexAdminPwd                         SET ErrorMessage= cannot find GexAdminPwd & goto Error

REM ============================================================
REM VERIFY IF HAVE PURGE 
REM ============================================================

SET NothingTodo=
IF NOT DEFINED GexPurgeSplitlotSamplesAfterNbDays AND NOT DEFINED GexPurgeSplitlotStatsAfterNbDays AND NOT DEFINED GexPurgeSplitlotEntryAfterNbDays         SET NothingTodo=1
IF DEFINED NothingTodo (@echo no purge configuration found in gexdb.ini)&(@echo End of process)&goto Error

REM ============================================================
@echo Beginning of the purge
IF DEFINED GexPurgeSplitlotSamplesAfterNbDays      @echo        purge samples after %GexPurgeSplitlotSamplesAfterNbDays% days
IF DEFINED GexPurgeSplitlotStatsAfterNbDays        @echo        purge stats after %GexPurgeSplitlotStatsAfterNbDays% days
IF DEFINED GexPurgeSplitlotEntryAfterNbDays        @echo        purge entry after %GexPurgeSplitlotEntryAfterNbDays% days
@echo Please wait ...

IF NOT DEFINED GexPurgeSplitlotSamplesAfterNbDays  SET GexPurgeSplitlotSamplesAfterNbDays=NULL
IF NOT DEFINED GexPurgeSplitlotStatsAfterNbDays    SET GexPurgeSplitlotStatsAfterNbDays=NULL
IF NOT DEFINED GexPurgeSplitlotEntryAfterNbDays    SET GexPurgeSplitlotEntryAfterNbDays=NULL
SET MySqlUser=--user=%GexAdminName% --password=%GexAdminPwd% --database=%GexDataBaseName%
mysql %MySqlUser% --execute="CALL PURGE_SPLITLOTS(%GexPurgeSplitlotSamplesAfterNbDays%, %GexPurgeSplitlotStatsAfterNbDays%, %GexPurgeSplitlotEntryAfterNbDays%)" 2>"%TempGalaxyFile%"
IF EXIST "%TempGalaxyFile%"                        SET /p a=<"%TempGalaxyFile%"
IF DEFINED a                                       SET ErrorMessage= mysql connection : %a%        & goto Error

REM ============================================================
@echo End of purge OK

goto Exit

REM ============================================================
REM EXIT SECTION
REM ============================================================

:Error
REM EndLocal
SET CallErrorMessage=%ErrorMessage%
IF DEFINED NoDisplay goto Exit
IF DEFINED CallErrorMessage                        @echo ERROR:%CallErrorMessage%

:Exit
IF DEFINED TempGalaxyFile del /q "%TempGalaxyFile%" 2>NUL
REM EndLocal
IF NOT DEFINED NoDisplay Pause

