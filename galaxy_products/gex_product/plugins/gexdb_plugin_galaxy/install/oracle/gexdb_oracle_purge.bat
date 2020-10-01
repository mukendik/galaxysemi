@echo OFF
SET CallErrorMessage=

SetLocal
SET ProgramName=%0
SET EchoOn=
SET PauseMode=
SET NoDisplay=
SET a=
SET TempGalaxyFile=

:AddCallParams
IF {%1}=={} goto FinAddCallParams
IF {%1}=={-echo} SET EchoOn=%1 & SHIFT & goto AddCallParams
IF {%1}=={-pause} SET PauseMode=%1 & SHIFT & goto AddCallParams
IF {%1}=={-nodisplay} SET NoDisplay=%1 & SHIFT & goto AddCallParams
@echo Usage :
@echo Usage : -echo : display all lines
@echo Usage : -pause : add some pause
@echo Usage : -nodisplay : for monitoring
goto Exit

:FinAddCallParams
IF DEFINED NoDisplay SET EchoOn= & SET PauseMode=
REM this program call the gexdb."PURGE_SPLITLOT_PROCEDURE" to purge and cleanup GexDb schema

SET TempGalaxyFile=%ProgramName%TempGalaxyFile.tmp
del /q "%TempGalaxyFile%" 2>NUL

@echo Read GexDb configuration file ...
call gexdb_oracle_readconfig.bat %EchoOn%  -gexdbini
IF DEFINED CallErrorMessage			SET ErrorMessage=%CallErrorMessage%		& goto Error

IF NOT DEFINED GexDbIni 			SET ErrorMessage= cannot find gexdb_oracle_install.ini in %datadir% & goto Error

REM if GexDb Backup dir not defined then ERROR : can't access to the backup server
IF NOT DEFINED GexAdminName 			SET ErrorMessage= cannot find GexAdminName & goto Error
IF NOT DEFINED GexAdminPwd			SET ErrorMessage= cannot find GexAdminPwd & goto Error

IF NOT DEFINED GexPurgeSplitlotSamplesAfterNbWeeks AND NOT DEFINED GexPurgeSplitlotStatsAfterNbWeeks AND NOT DEFINED GexPurgeSplitlotEntriesAfterNbWeeks 	SET NothingTodo=1
IF DEFINED NothingTodo (@echo no purge configuration found in gexdb_oracle_install.ini)&(@echo End of process)&goto Error

@echo Begining of the purge
IF DEFINED GexPurgeSplitlotSamplesAfterNbWeeks 	@echo        purge samples after %GexPurgeSplitlotSamplesAfterNbWeeks% weeks
IF DEFINED GexPurgeSplitlotStatsAfterNbWeeks 	@echo        purge stats after %GexPurgeSplitlotStatsAfterNbWeeks% weeks
IF DEFINED GexPurgeSplitlotEntriesAfterNbWeeks 	@echo        purge entries after %GexPurgeSplitlotEntriesAfterNbWeeks% weeks
@echo Please wait ...

echo -- CALL %GexAdminName%."PURGE_SPLITLOT"() >"%TempGalaxyFile%.sql"
IF NOT DEFINED GexPurgeSplitlotSamplesAfterNbWeeks	goto LabelEndSamples
echo CALL %GexAdminName%."PURGE_SPLITLOT"('ft',%GexPurgeSplitlotSamplesAfterNbWeeks%,1); >>"%TempGalaxyFile%.sql"
echo CALL %GexAdminName%."PURGE_SPLITLOT"('et',%GexPurgeSplitlotSamplesAfterNbWeeks%,1); >>"%TempGalaxyFile%.sql"
echo CALL %GexAdminName%."PURGE_SPLITLOT"('wt',%GexPurgeSplitlotSamplesAfterNbWeeks%,1); >>"%TempGalaxyFile%.sql"
:LabelEndSamples

IF NOT DEFINED GexPurgeSplitlotStatsAfterNbWeeks 	goto LabelEndStats
echo CALL %GexAdminName%."PURGE_SPLITLOT"('ft',%GexPurgeSplitlotStatsAfterNbWeeks%,2); >>"%TempGalaxyFile%.sql"
echo CALL %GexAdminName%."PURGE_SPLITLOT"('et',%GexPurgeSplitlotStatsAfterNbWeeks%,2); >>"%TempGalaxyFile%.sql"
echo CALL %GexAdminName%."PURGE_SPLITLOT"('wt',%GexPurgeSplitlotStatsAfterNbWeeks%,2); >>"%TempGalaxyFile%.sql"
:LabelEndStats

IF NOT DEFINED GexPurgeSplitlotEntriesAfterNbWeeks 	goto LabelEndEntries
echo CALL %GexAdminName%."PURGE_SPLITLOT"('ft',%GexPurgeSplitlotEntriesAfterNbWeeks%,3); >>"%TempGalaxyFile%.sql"
echo CALL %GexAdminName%."PURGE_SPLITLOT"('et',%GexPurgeSplitlotEntriesAfterNbWeeks%,3); >>"%TempGalaxyFile%.sql"
echo CALL %GexAdminName%."PURGE_SPLITLOT"('wt',%GexPurgeSplitlotEntriesAfterNbWeeks%,3); >>"%TempGalaxyFile%.sql"
:LabelEndEntries

echo exit; >>"%TempGalaxyFile%.sql"

SET OracleUser=%GexAdminName%/%GexAdminPwd%@%GexDataBaseInstance%
IF DEFINED PauseMode Pause
(sqlplus %OracleUser% @"%TempGalaxyFile%.sql")>"%TempGalaxyFile%"
IF DEFINED PauseMode Pause

IF EXIST "%TempGalaxyFile%.sql" 	del /q "%TempGalaxyFile%.sql" 2>NUL
IF EXIST "%TempGalaxyFile%"		for /f "delims=" %%a in ('type "%TempGalaxyFile%"^|find "ORA-"') do SET a=%%a
IF DEFINED a SET ErrorMessage= mysql connection : %a%	& goto Error

@echo End of purge OK

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

