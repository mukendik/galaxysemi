@echo OFF
cls
SET ProgramName=%0
SET EchoOn=
SET PauseMode=
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
REM this program call the MySql Check&Repair for gexdb

mySqlCheck --silent --auto-repair galaxu_db

:Exit
IF NOT DEFINED NoDisplay Pause