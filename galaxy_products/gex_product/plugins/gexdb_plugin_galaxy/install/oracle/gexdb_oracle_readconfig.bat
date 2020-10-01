@echo OFF
SET CallErrorMessage=
SET EchoOn=
SET ReadGexDbIni=

:AddCallParams
IF {%1}=={} 			goto FinAddCallParams
IF {%1}=={-echo} 		SET EchoOn=%1 & SHIFT & goto AddCallParams
IF {%1}=={-gexdbini} 		SET ReadGexDbIni=1 & SHIFT & goto AddCallParams
@echo Usage :
@echo Usage : -echo 		: display all lines
@echo Usage : -gexdbini 	: read gexdb_oracle_install.ini
goto :eof
:FinAddCallParams

IF DEFINED EchoON @echo ON

REM this program only read the GexDb Config Files

IF NOT DEFINED ReadGexDbIni goto EndReadGexDbIni

SET GexDataBaseName=
SET GexAdminName=
SET GexAdminPwd=
SET GexUserName=
SET GexUserPwd=
SET GexHotBackupName=
SET GexHotBackupPwd=
SET GexHotBackupDir=
SET GexHotBackupHour=
SET GexHotBackupEvery=
SET GexBackupDir=
SET GexPurgeSplitlotSamplesAfterNbWeeks=
SET GexPurgeSplitlotStatsAfterNbWeeks=
SET GexPurgeSplitlotEntriesAfterNbWeeks=
SET GexPurgeHour=
SET GexPurgeEvery=
SET GexBackupIncrementalHour=
SET GexBackupIncrementalEvery=
SET GexBackupTotalHour=
SET GexBackupTotalEvery=


IF NOT DEFINED GexDbIni		SET GexDbIni=gexdb_oracle_install.ini
IF NOT EXIST "%GexDbIni%" 	SET GexDbIni=
IF NOT DEFINED GexDbIni		SET ErrorMessage= cannot find gexdb_oracle_install.ini in %CD% & goto Error

REM SEARCH THE GEXDB CONFIGURATION GEXDB.INI STORED IN BIN DIR WITH THE GEXDB SCRIPT
REM ALL DATA IN THIS FILE HAVE TO BE SET
for /f "delims=" %%a in ('type "%GexDbIni%"^|find "="^|find /v "#"^|find /v "<"^|find /v ">"') do SET %%a

IF NOT DEFINED GexBackupDir	goto EndFormatBackupDir
REM HAVE FOUND A VALUE FOR GEXBACKUPDIR : FORMAT IT
REM REPLACE " WITH §
SET GexBackupDir=%GexBackupDir:"=§%
CALL :SubRoutineFormatPath "%GexBackupDir%"
SET GexBackupDir=%Result:§=%
:EndFormatBackupDir

IF NOT DEFINED GexHotBackupDir	goto EndFormatHotBackupDir
REM HAVE FOUND A VALUE FOR GEXHOTBACKUPDIR : FORMAT IT
REM REPLACE " WITH §
SET GexHotBackupDir=%GexHotBackupDir:"=§%
CALL :SubRoutineFormatPath "%GexHotBackupDir%"
SET GexHotBackupDir=%Result:§=%
:EndFormatHotBackupDir

:EndReadGexDbIni

SET DateDir=%date:/=%
for /f %%a in ('echo ^|date ^| find "jj-mm"') do SET DateDir=%date:~-4%%date:~-7,2%%date:~-10,2%& goto EndReadConfig
for /f %%a in ('echo ^|date ^| find "mm-jj"') do SET DateDir=%date:~-4%%date:~-10,2%%date:~-7,2%& goto EndReadConfig
for /f %%a in ('echo ^|date ^| find "dd-mm"') do SET DateDir=%date:~-4%%date:~-7,2%%date:~-10,2%& goto EndReadConfig
for /f %%a in ('echo ^|date ^| find "mm-dd"') do SET DateDir=%date:~-4%%date:~-10,2%%date:~-7,2%& goto EndReadConfig


:EndReadConfig

goto :Exit

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

:Error
SET CallErrorMessage=%ErrorMessage%

:Exit
