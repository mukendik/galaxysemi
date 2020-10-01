@echo OFF
SET CallErrorMessage=
cls
SET ProgramName=%0
SET EchoOn=
SET PauseMode=
SET a=
SET TempGalaxyFile=gexdb_installTempGalaxyFile.tmp


REM ============================================================
REM GEXDB_INSTALL OPTIONS
REM ============================================================

:AddCallParams
IF {%1}=={}              goto FinAddCallParams
IF {%1}=={-echo}         SET EchoOn=%1 & SHIFT & goto AddCallParams
IF {%1}=={-pause}        SET PauseMode=%1 & SHIFT & goto AddCallParams
IF {%1}=={-nodisplay}    SET NoDisplay=%1 & SHIFT & goto AddCallParams
@echo Usage :
@echo Usage : -echo      : display all lines
@echo Usage : -pause     : add some pause
@echo Usage : -nodisplay : for monitoring
goto Exit

:FinAddCallParams
IF DEFINED NoDisplay SET EchoOn= & SET PauseMode=

IF DEFINED EchoON @echo ON
REM this program install the GexDb for master and slave

IF DEFINED TempGalaxyFile del /q "%TempGalaxyFile%" 2>NUL

@echo.
@echo ***************************************
@echo *                                     *
@echo *      Galaxy GexDB SQL database      *
@echo *                                     *
@echo ***************************************

REM ============================================================
REM READ GEXDB CONFIGURATION
REM USE SCRIPT SQL FOR CREATE GEXDBUSER
REM USE SCRIPT SQL FOR CREATE GEXDB WITH PROCEDURES
REM ASK FOR SCHEDULER TASK (TOTAL BACKUP, INCR BACKUP, PURGE)
REM ============================================================

REM ============================================================
REM READ GexServerStatus
REM ============================================================

SET GexServerStatus=
IF NOT EXIST "gexdb_mysql_install_b9.sql"      SET ErrorMessage= cannot find gexdb_mysql_install_b9.sql in %CD% & goto Error
IF NOT EXIST "gexdb_mysql_install.ini"         SET ErrorMessage= cannot find gexdb_mysql_install.ini in %CD% & goto Error
for /f "delims=" %%a in ('type "gexdb_mysql_install.ini"^|find "GexServerStatus"^|find /v "<"^|find /v ">"') do SET %%a
IF NOT DEFINED GexServerStatus                 SET GexServerStatus=master
IF "%GexServerStatus%"=="master"               @echo *    MASTER DATABASE INSTALLATION     *
IF NOT "%GexServerStatus%"=="master"           @echo *    SLAVE DATABASE INSTALLATION      *

@echo ***************************************

IF DEFINED PauseMode PAUSE

REM ============================================================
REM READ MYSQL CONFIGURATION FILE
REM ============================================================

@echo.
@echo o Reading MySql configuration file...

SET MySqlIni=
call gexdb_mysql_readconfig.bat %EchoOn% -myini
IF DEFINED CallErrorMessage                    SET ErrorMessage=%CallErrorMessage%         & goto Error

IF DEFINED PauseMode PAUSE

REM if mysql datadir not defined then ERROR : can't access to the bin-log information
IF NOT DEFINED MySqlIni                        SET ErrorMessage= cannot find my.ini in %CD%\..\ & goto Error
IF NOT DEFINED datadir                         SET ErrorMessage= no datadir defined in my.ini & goto Error

REM ============================================================
REM COPY AND READ GEXDB CONFIGURATION FILE
REM ============================================================

@echo.
@echo o Copying GexDb configuration file...

IF EXIST "gexdb.ini"                del /q "gexdb.ini"
COPY "gexdb_mysql_install.ini" "gexdb.ini"2>"%TempGalaxyFile%" >NUL
SET a=
IF EXIST "%TempGalaxyFile%" SET /p a=<"%TempGalaxyFile%"
IF DEFINED TempGalaxyFile del /q "%TempGalaxyFile%" 2>NUL
IF DEFINED a ATTRIB -R "gexdb.ini"2>NUL >NUL & COPY "gexdb_mysql_install.ini" "gexdb.ini"2>"%TempGalaxyFile%" >NUL
SET a=
IF EXIST "%TempGalaxyFile%" SET /p a=<"%TempGalaxyFile%"
IF DEFINED a SET ErrorMessage= copy gexdb.ini : %a%        & goto Error

IF DEFINED PauseMode PAUSE

@echo.
@echo o Reading GexDb configuration file...

call gexdb_mysql_readconfig.bat %EchoOn% -gexdbini
IF DEFINED CallErrorMessage                    SET ErrorMessage=%CallErrorMessage%                        & goto Error

IF DEFINED PauseMode PAUSE

IF NOT DEFINED GexDbIni                        SET ErrorMessage= cannot find gexdb.ini in %datadir% & goto Error
IF NOT DEFINED GexAdminName                    SET ErrorMessage= GexDb Administrator not defined         & goto Error
IF NOT DEFINED GexUserName                     SET ErrorMessage= GexDb User not defined                 & goto Error
IF "%GexServerStatus%"=="slave"  IF NOT DEFINED GexReplicationName              SET ErrorMessage= GexDb Replication not defined         & goto Error
REM IF NOT DEFINED GexBackupDir                SET ErrorMessage= GexBackupDir not defined                & goto Error
IF NOT DEFINED GexServerStatus                 SET ErrorMessage= GexServerStatus not defined                & goto Error

IF "%GexServerStatus%"=="slave" IF "%GexServerId%"=="1" (@echo   WARNING: Slave with server-id=1, server-id is set to 2)&SET GexServerId=2

REM ============================================================
REM ASK ROOT NAME AND PASSWORD FOR GEXDB INSTALLATION
REM ============================================================

@echo.
@echo o Please enter root login details:

SET NbTry=0
:AskForRoot
SET RootName=
SET RootPwd=
SET a=
SET /a NbTry=%NbTry%+1
SET /p RootName=  User     : 
SET /p  RootPwd=  Password : 
SET MySqlRoot=--user=%RootName% --password=%RootPwd%
REM Test conection
mysql %MySqlRoot% --execute="SHOW DATABASES" 2>"%TempGalaxyFile%" >NUL
IF EXIST "%TempGalaxyFile%" SET /p a=<"%TempGalaxyFile%"
IF DEFINED a @echo   ******* MySql connection: cannot connect with user=%RootName% and password=%RootPwd%
IF DEFINED a @echo   ******* %a%
IF DEFINED a IF 2 lss %NbTry%                  SET ErrorMessage= cannot connect to MySql Server        & goto Error
IF DEFINED a goto AskForRoot

REM ============================================================
REM GEXDB DATABASE CREATION
REM ============================================================

:DataBaseCreation
SET GexDbSql=gexdb_mysql_install_b9.sql
SET GexDataBaseName=%GexAdminName%

IF DEFINED PauseMode PAUSE

IF DEFINED TempGalaxyFile del /q "%TempGalaxyFile%" 2>NUL
REM CREATE THE NEW GEXDB

REM ============================================================
REM CREATE AN EMPTY GEXDB
REM ============================================================

@echo.
@echo o Creating GexDb database...

mysql %MySqlRoot% --execute="DROP DATABASE %GexDataBaseName%" 2>NUL
mysql %MySqlRoot% --execute="CREATE DATABASE %GexDataBaseName%" 2>"%TempGalaxyFile%"
IF EXIST "%TempGalaxyFile%" SET /p a=<"%TempGalaxyFile%"
IF DEFINED a SET ErrorMessage= GexDb Database creation fail : %a%        & goto Error

IF DEFINED PauseMode PAUSE

@echo.
@echo o Creating GexDb users...

REM ============================================================
REM CREATE GEXDB USERS
REM ============================================================

mysql %MySqlRoot% --execute="DROP USER '%GexAdminName%'@'%%'" 2>NUL

mysql %MySqlRoot% --execute="DROP USER '%GexUserName%'@'%%'"  2>NUL

IF DEFINED GexReplicationName (mysql %MySqlRoot% --execute="DROP USER '%GexReplicationName%'@'%%'" 2>NUL)

IF DEFINED PauseMode PAUSE

mysql %MySqlRoot% --execute="GRANT ALL ON %GexDataBaseName%.* TO '%GexAdminName%'@'%%' IDENTIFIED BY '%GexAdminPwd%'" 2>"%TempGalaxyFile%"
IF EXIST "%TempGalaxyFile%" SET /p a=<"%TempGalaxyFile%"
IF DEFINED a SET ErrorMessage= cannot create user %GexAdminName%: %a%        & goto Error

mysql %MySqlRoot% --execute="GRANT FILE ON *.* TO '%GexAdminName%'@'%%' IDENTIFIED BY '%GexAdminPwd%'" 2>"%TempGalaxyFile%"
IF EXIST "%TempGalaxyFile%" SET /p a=<"%TempGalaxyFile%"
IF DEFINED a SET ErrorMessage= cannot create user %GexAdminName%: %a%        & goto Error

mysql %MySqlRoot% --execute="GRANT SELECT, SHOW VIEW, CREATE VIEW, CREATE TEMPORARY TABLES ON %GexDataBaseName%.* TO '%GexUserName%'@'%%' IDENTIFIED BY '%GexUserPwd%'" 2>"%TempGalaxyFile%"
IF EXIST "%TempGalaxyFile%" SET /p a=<"%TempGalaxyFile%"
IF DEFINED a SET ErrorMessage= cannot create user %GexUserName%: %a%        & goto Error

IF DEFINED GexReplicationName (mysql %MySqlRoot% --execute="GRANT REPLICATION SLAVE, REPLICATION CLIENT ON *.* TO '%GexReplicationName%'@'%%' IDENTIFIED BY '%GexReplicationPwd%'" 2>"%TempGalaxyFile%")
IF EXIST "%TempGalaxyFile%" SET /p a=<"%TempGalaxyFile%"
IF DEFINED a SET ErrorMessage= cannot create user %GexReplicationName%: %a%        & goto Error

mysql %MySqlRoot% --execute="FLUSH PRIVILEGES" 2>NUL


IF DEFINED PauseMode PAUSE

@echo.
@echo o Creating GexDb tables...

REM ============================================================
REM CREATE ALL GEXDB TABLES
REM ============================================================

SET MySqlAdmin=--user=%GexAdminName% --password=%GexAdminPwd%
mysql %MySqlAdmin% --execute="show databases" 2>"%TempGalaxyFile%" >NUL
IF EXIST "%TempGalaxyFile%" SET /p a=<"%TempGalaxyFile%"
IF DEFINED a SET ErrorMessage= GexDb_admin connection fail: %a%        & goto Error

(mysql %MySqlAdmin% %GexDataBaseName% < "%GexDbSql%") 2>"%TempGalaxyFile%"
IF EXIST "%TempGalaxyFile%" SET /p a=<"%TempGalaxyFile%"
IF DEFINED a SET ErrorMessage= GexDb tables creation fail: %a%        & goto Error


REM SKIP THIS STEP FOR THE MOMENT
REM goto RestartServer

IF "%GexCreateDbOnly%"=="1"                goto EndInstall
IF DEFINED PauseMode PAUSE

REM ============================================================
REM GEXDB SCHEDULER
REM ============================================================

IF NOT EXIST "gexdb_mysql_scheduler.bat"             goto EndScheduler
CALL gexdb_mysql_scheduler.bat %EchoOn% -install -noreadconfig -nodisplay
IF DEFINED CallErrorMessage                    SET ErrorMessage=%CallErrorMessage%        & goto Error
:EndScheduler

IF NOT "%GexServerStatus%"=="master"           goto RestartServer
IF NOT DEFINED GexBackupDir                    goto RestartServer
REM ============================================================
REM CREATE THE FIRST BACKUP
REM ============================================================

REM if GexBackup dir not defined then ERROR : can't access to the backup server

@echo.
@echo o Saving the first GexDb dump in the backup directory...

IF DEFINED TempGalaxyFile del /q "%TempGalaxyFile%" 2>NUL

IF NOT EXIST "%GexBackupDir%"                  SET ErrorMessage= %GexBackupDir% doesn't exist        & goto Error
IF EXIST "%GexBackupDir%%DateDir%"             del /q "%GexBackupDir%%DateDir%\*.zip" 2>NUL
IF NOT EXIST "%GexBackupDir%%DateDir%"         SET ErrorMessage= %GexBackupDir%%DateDir% doesn't exist        & goto Error

zip -D "%GexDbSql%.zip" "%GexDbSql%" >NUL
copy /Y "%GexDbSql%.zip" "%GexBackupDir%%DateDir%">NUL
del /q "%GexDbSql%.zip" 2>NUL


IF DEFINED PauseMode PAUSE
:RestartServer

IF NOT DEFINED GexBinLogName                   goto EndInstall

REM ============================================================
REM INSERT LINE IN MY.INI FOR GEXDB CONFIGURATION
REM ============================================================

@echo.
@echo o Writing MySql configuration file...

SET HaveComment=1
IF DEFINED TempGalaxyFile del /q "%TempGalaxyFile%" 2>NUL
FOR /f "delims=" %%a in ('type "%MySqlIni%"') do (SET Params=%%a)&call :SubRoutineWriteFile


IF DEFINED PauseMode PAUSE


@echo.
@echo o Restarting server for bin-log configuration...

REM ============================================================
REM VERIFY IF OK BEFORE STOP SERVER
IF NOT EXIST "%TempGalaxyFile%"                 @echo   ERROR during the generation of the new my.ini        & goto Error
net stop mysql >NUL

del /q "%MySqlIni%.%DateDir%.bak" 2>NUL >NUL
copy "%MySqlIni%" "%MySqlIni%.%DateDir%.bak" 2>NUL >NUL
del /q "%MySqlIni%" 2>NUL
copy "%TempGalaxyFile%" "%MySqlIni%">NUL
del /q "%TempGalaxyFile%" 2>NUL

REM ============================================================
REM CLEAN THE SERVER AND ZIP AND BACKUP THE SQL DUMP
IF NOT DEFINED GexBackupDir                     goto EndBackupDir

del /q "%datadir%*.00*" 2>NUL
del /q "%datadir%*.index" 2>NUL
:EndBackupDir

REM ============================================================
REM restart server
net start mysql 2>"%TempGalaxyFile%"
IF EXIST "%TempGalaxyFile%" SET /p a=<"%TempGalaxyFile%"
IF DEFINED a (@echo ERROR: mysql server : %a%)
IF DEFINED a (@echo Restore last version ...)
IF DEFINED a (del /q "%MySqlIni%" 2>NUL) & (copy "%MySqlIni%.%DateDir%.bak" "%MySqlIni%">NUL)
IF DEFINED a (net start mysql) & SET ErrorMessage= mysql server : %a% & goto Error

IF "%GexServerStatus%"=="slave" mysql %MySqlRoot% --execute="stop slave"
mysql %MySqlRoot% --execute="reset %GexServerStatus%"
IF "%GexServerStatus%"=="slave" mysql %MySqlRoot% --execute="start slave"

REM ============================================================
REM END OF GEXDB INSTALLATION
REM ============================================================

:EndInstall

@echo.
@echo **** Success: GexDb database created
@echo.
@echo      GexDb database = %GexDataBaseName%
@echo      GexDb admin    = %GexAdminName%
@echo      GexDb user     = %GexUserName%

goto Exit

REM ============================================================
REM SUBROUTINE SECTION
REM ============================================================

:SubRoutineWriteFile
SET MyId=%Params%

REM REPLACE " by §
SET MyId=%MyId:"=§%

REM Supp space at end of the line
:SupprEndSpace
IF NOT "%MyId:~-1%"==" "                         goto FinSupprEndSpace
SET MyId=%MyId:~0,-1%
goto SupprEndSpace
:FinSupprEndSpace

IF "%MyId:~1,7%"==" GexDb"                       goto :eof
REM INSERT LINE BEFORE [chapter]
REM INSERT LINE BEFORE # comment
IF "%MyId:~0,1%"=="["                            (@echo\)>>%TempGalaxyFile% & (@echo\)>>"%TempGalaxyFile%
IF "%MyId:~0,1%"=="#" IF NOT DEFINED HaveComment SET HaveComment=1 & (@echo\)>>%TempGalaxyFile%
IF NOT "%MyId:~0,1%"=="#" SET HaveComment=

IF "%MyId:~0,1%"=="#"                             @echo %MyId:§="% >>"%TempGalaxyFile%" & goto :eof
IF "%MyId:~1,6%"=="mysqld"                       (@echo %MyId:§="%)>>"%TempGalaxyFile%" & goto InsertData
IF "%MyId:~0,1%"=="["                            (@echo %MyId:§="%)>>"%TempGalaxyFile%" & goto :eof

SET Skip=
IF NOT "%GexServerStatus%"=="master" IF NOT DEFINED GexBackupIncrementalEvery SET Skip=#

IF "%MyId:~0,7%"=="log-bin"                      (@echo %Skip%log-bin=%GexBinLogName%)>>"%TempGalaxyFile%" & goto :eof
IF "%MyId:~0,10%"=="server-id="                  (@echo server-id=%GexServerId%)>>"%TempGalaxyFile%" & goto :eof
IF "%MyId:~0,13%"=="binlog-do-db="               (@echo binlog-do-db=%GexDataBaseName%)>>"%TempGalaxyFile%" & goto :eof

SET Skip=
IF "%GexServerStatus%"=="master" SET Skip=#

IF "%MyId:~0,12%"=="master-host="                (@echo %Skip%master-host=%GexMasterHost%)>>"%TempGalaxyFile%" & goto :eof
IF "%MyId:~0,12%"=="master-port="                (@echo %Skip%master-port=%GexMasterPort%)>>"%TempGalaxyFile%" & goto :eof
IF "%MyId:~0,12%"=="master-user="                (@echo %Skip%master-user=%GexReplicationName%)>>"%TempGalaxyFile%" & goto :eof
IF "%MyId:~0,16%"=="master-password="            (@echo %Skip%master-password=%GexReplicationPwd%)>>"%TempGalaxyFile%" & goto :eof
IF "%MyId:~0,16%"=="replicate-do-db="            (@echo %Skip%replicate-do-db=%GexDataBaseName%)>>"%TempGalaxyFile%" & goto :eof

(@echo %MyId:§="%)>>%TempGalaxyFile%
goto :eof

:InsertData

(@echo # GexDb configuration for %GexServerStatus%)>>"%TempGalaxyFile%"
SET Skip=
IF NOT "%GexServerStatus%"=="master" IF NOT DEFINED GexBackupIncrementalEvery SET Skip=#

IF NOT DEFINED server-id                         (@echo server-id=%GexServerId%)>>"%TempGalaxyFile%"
IF NOT DEFINED log-bin                           (@echo %Skip%log-bin=%GexBinLogName%)>>"%TempGalaxyFile%"
IF NOT DEFINED binlog-do-db                      (@echo binlog-do-db=%GexDataBaseName%)>>"%TempGalaxyFile%"

IF "%GexServerStatus%"=="master" goto EndSubRoutine
IF NOT DEFINED master-host                       (@echo master-host=%GexMasterHost%)>>"%TempGalaxyFile%"
IF NOT DEFINED master-port                       (@echo master-port=%port%)>>"%TempGalaxyFile%"
IF NOT DEFINED master-user                       (@echo master-user=%GexReplicationName%)>>"%TempGalaxyFile%"
IF NOT DEFINED master-password                   (@echo master-password=%GexReplicationPwd%)>>"%TempGalaxyFile%"
IF NOT DEFINED replicate-do-db                   (@echo replicate-do-db=%GexDataBaseName%)>>"%TempGalaxyFile%"
:EndSubRoutine

goto :eof

REM ============================================================
REM EXIT SECTION
REM ============================================================

:Error
SET CallErrorMessage=%ErrorMessage%
IF DEFINED CallErrorMessage                      @echo   ERROR:%CallErrorMessage%
@echo.

:Exit
IF DEFINED TempGalaxyFile del /q "%TempGalaxyFile%" 2>NUL
IF NOT DEFINED NoDisplay Pause