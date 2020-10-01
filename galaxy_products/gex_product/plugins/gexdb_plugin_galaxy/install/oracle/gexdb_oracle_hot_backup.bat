@echo OFF
SET CallErrorMessage=
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
REM this program install the GexDb total backup
REM if the instant is in NOARCHIVELOG, do a cold backup
REM else do a hot backup

IF DEFINED TempGalaxyFile 		del /q "%TempGalaxyFile%" 2>NUL

REM READ GEXDB CONFIGURATION

SET ArchiveLogMode=
SET TableSpaceName=
SET CopyName=
SET UDump=
SET ProcessId=

SET LogFile=

SET GexDbIni=gexdb_oracle_install.ini
IF NOT EXIST "%GexDbIni%"			SET ErrorMessage= cannot find %GexDbIni% in %CD% & goto Error

IF DEFINED PauseMode (echo ================PAUSE=================)&PAUSE

@echo Read GexDb configuration file ...
call gexdb_oracle_readconfig.bat %EchoOn% -gexdbini
IF DEFINED CallErrorMessage			SET ErrorMessage=%CallErrorMessage%			& goto Error

IF DEFINED PauseMode (echo ================PAUSE=================)&PAUSE

IF NOT DEFINED GexDbIni 			SET ErrorMessage= cannot find gexdb.ini in %datadir% 	& goto Error
IF NOT DEFINED GexDataBaseInstance		SET ErrorMessage= Oracle instance not defined 	& goto Error
IF NOT DEFINED GexHotBackupName 		SET ErrorMessage= GexHotBackup Administrator not defined 	& goto Error
IF NOT DEFINED GexHotBackupDir 			SET ErrorMessage= GexHotBackupDir not defined		& goto Error
IF DEFINED GexHotBackupDeleteArchives		IF NOT %GexHotBackupDeleteArchives%==1	SET GexHotBackupDeleteArchives=
SET IncludeReadOnly=1
SET OradimUsrPwd=-USRPWD %GexHotBackupPwd% 

SET SqlConnect=sqlplus -L %GexHotBackupName%/%GexHotBackupPwd%@%GexDataBaseInstance%
SET BackupDir=%GexHotBackupDir%

SET Logfile=%BackupDir%%GexDataBaseInstance%_hotbackuplog_%DateDir%.log
echo Beginning Backup at %TIME% > %LogFile% 

goto Main

:SubRoutineExecSql
IF DEFINED CallErrorMessage 	goto :eof
(%SqlConnect% @%OutputFile%)>"%TempGalaxyFile%"
SET a=
for /f "delims=" %%a in ('type "%TempGalaxyFile%"^|find /V "ORA-06512"^|find /I "ORA-"') do SET a=%%a
IF DEFINED PauseMode (echo ================PAUSE=================)&PAUSE
IF DEFINED TempGalaxyFile 		del /q "%TempGalaxyFile%" 2>NUL
IF DEFINED a SET ErrorMessage= Sql fail : %a%	& goto Error

goto :eof

:SubRoutineOracleInfo
IF DEFINED CallErrorMessage 	goto :eof

SET ReadOnlyClause=
IF NOT DEFINED IncludeReadOnly 		SET ReadOnlyClause= and v.ENABLED='READ WRITE'

SET OutputFile=%BackupDir%OracleInfo.sql
echo set trimspool on;>%OutputFile%
echo set heading off;>>%OutputFile%
echo set feedback off;>>%OutputFile%
echo set echo off;>>%OutputFile%
echo spool %BackupDir%archivemode.txt;>>%OutputFile%
echo select log_mode from v$database;>>%OutputFile%
echo spool off;>>%OutputFile%
echo spool %BackupDir%instancename.txt;>>%OutputFile%
echo select instance_name from v$instance;>>%OutputFile%
echo spool off;>>%OutputFile%
echo spool %BackupDir%datafiles.txt;>>%OutputFile%
echo select name from v$datafile v where (STATUS='ONLINE' OR STATUS='SYSTEM')%ReadOnlyClause%;>>%OutputFile%
echo spool off;>>%OutputFile%
echo spool %BackupDir%controlfiles.txt;>>%OutputFile%
echo select name from v$controlfile;>>%OutputFile%
echo spool off;>>%OutputFile%
echo spool %BackupDir%tablespaces.txt;>>%OutputFile%
echo select TABLESPACE_NAME from DBA_TABLESPACES where STATUS in ('ONLINE','READ ONLY') and CONTENTS in ('PERMANENT','UNDO');>>%OutputFile%
echo spool off;>>%OutputFile%
echo exit;>>%OutputFile%

CALL :SubRoutineExecSql
IF DEFINED CallErrorMessage 	goto :eof

IF DEFINED PauseMode (echo ================PAUSE=================)&PAUSE
goto :eof

:SubRoutineCheckArchivemode
IF DEFINED CallErrorMessage 	goto :eof

SET InputFile=%BackupDir%archivemode.txt
SET a=
for /f "delims=" %%a in ('type "%InputFile%"') do SET a=%%a
If "%a%"=="ARCHIVELOG" SET ArchiveLogMode=1

IF DEFINED PauseMode (echo ================PAUSE=================)&PAUSE
goto :eof


:SubRoutineCheckInstance
IF DEFINED CallErrorMessage 	goto :eof

SET InputFile=%BackupDir%instancename.txt
SET a=
for /f "delims=" %%a in ('type "%InputFile%"') do SET InstanceName=%%a
 
IF DEFINED PauseMode (echo ================PAUSE=================)&PAUSE
goto :eof

:SubRoutineLogSwitch
IF DEFINED CallErrorMessage 	goto :eof

echo Issuing Switch Logfile command at %DATE% - %TIME%.>>%LogFile%

SET OutputFile=%BackupDir%logswitch.sql
echo alter system switch logfile;>%OutputFile%
echo exit;>>%OutputFile%

CALL :SubRoutineExecSql
IF DEFINED CallErrorMessage 	goto :eof

IF DEFINED PauseMode (echo ================PAUSE=================)&PAUSE
goto :eof

 
:SubRoutineShutInstance
IF DEFINED CallErrorMessage 	goto :eof

@echo Shutting down the instance at %DATE% - %TIME%. 
echo Shutting down the instance at %DATE% - %TIME%.>>%LogFile% 
(oradim -SHUTDOWN -SID %InstanceName% %OradimUsrPwd% -SHUTTYPE srvc,inst -SHUTMODE IMMEDIATE)>"%TempGalaxyFile%"
SET a=
for /f "delims=" %%a in ('type "%TempGalaxyFile%"^|find /V "ORA-06512"^|find /I "ORA-"') do SET a=%%a
IF DEFINED PauseMode (echo ================PAUSE=================)&PAUSE
IF DEFINED TempGalaxyFile 		del /q "%TempGalaxyFile%" 2>NUL
IF DEFINED a SET ErrorMessage= oradim shutdown fail : %a%	& goto Error


IF DEFINED PauseMode (echo ================PAUSE=================)&PAUSE
goto :eof

:SubRoutineStartInstance
IF DEFINED CallErrorMessage 	goto :eof

@echo Starting up the instance at %DATE% - %TIME%.
echo Starting up the instance at %DATE% - %TIME%.>>%LogFile% 
(oradim -STARTUP -SID %InstanceName% %OradimUsrPwd% -STARTTYPE srvc)>"%TempGalaxyFile%"
SET a=
for /f "delims=" %%a in ('type "%TempGalaxyFile%"^|find /V "ORA-06512"^|find /I "ORA-"') do SET a=%%a
IF DEFINED PauseMode (echo ================PAUSE=================)&PAUSE
IF DEFINED TempGalaxyFile 		del /q "%TempGalaxyFile%" 2>NUL
IF DEFINED a SET ErrorMessage= oradim start fail : %a%	& goto Error

(oradim -STARTUP -SID %InstanceName% %OradimUsrPwd% -STARTTYPE inst)>"%TempGalaxyFile%"
SET a=
for /f "delims=" %%a in ('type "%TempGalaxyFile%"^|find /V "ORA-06512"^|find /I "ORA-"') do SET a=%%a
IF DEFINED PauseMode (echo ================PAUSE=================)&PAUSE
IF DEFINED TempGalaxyFile 		del /q "%TempGalaxyFile%" 2>NUL
IF DEFINED a SET ErrorMessage= oradim start fail : %a%	& goto Error


IF DEFINED PauseMode (echo ================PAUSE=================)&PAUSE
goto :eof

:SubRoutineCopyDataFiles
IF DEFINED CallErrorMessage 	goto :eof

SET CopyDir=%BackupDir%%GexDatabaseInstance%%DateDir%\
mkdir %CopyDir% 2>NUL >NUL

@echo Copy datafiles ...
SET Inputfile=%BackupDir%datafiles.txt
for /f "delims=" %%a in ('type "%InputFile%"') do (copy %%a %CopyDir%>NUL)&(echo Begining a cold copy of %%a.>>%LogFile%) 

IF DEFINED PauseMode (echo ================PAUSE=================)&PAUSE
goto :eof

:SubRoutineBeginBackup
IF DEFINED CallErrorMessage 	goto :eof

SET OutputFile=%BackupDir%beginbackup.sql
echo alter tablespace %TableSpaceName% begin backup;>%OutputFile%
echo exit;>>%OutputFile%

echo Putting tablespace %TableSpaceName% into hot backup mode at %DATE% - %TIME%.>>%LogFile% 

CALL :SubRoutineExecSql
IF DEFINED CallErrorMessage 	goto :eof

IF DEFINED PauseMode (echo ================PAUSE=================)&PAUSE
goto :eof

:SubRoutineEndBackup
IF DEFINED CallErrorMessage 	goto :eof

SET OutputFile=%BackupDir%endbackup.sql
echo alter tablespace %TableSpaceName% end backup;>%OutputFile%
echo exit;>>%OutputFile%

echo Taking tablespace %TableSpaceName% out of hot backup mode at %DATE% - %TIME%.>>%LogFile%

CALL :SubRoutineExecSql
IF DEFINED CallErrorMessage 	goto :eof

IF DEFINED PauseMode (echo ================PAUSE=================)&PAUSE
goto :eof

:SubRoutineHotCopyTableSpace
IF DEFINED CallErrorMessage 	goto :eof

@echo Copy datafiles for tablespace %TableSpaceName%
SET OutputFile=%BackupDir%hotfiles.sql

echo set trimspool on;>%OutputFile%
echo set heading off;>>%OutputFile%
echo set feedback off;>>%OutputFile%
echo set echo off;>>%OutputFile%
echo spool %BackupDir%hotfiles.txt>>%OutputFile%
echo select v.name from V$DATAFILE v, V$TABLESPACE vt, DBA_DATA_FILES d, DBA_TABLESPACES t>>%OutputFile%
echo where v.ts#=vt.ts# and vt.name=d.tablespace_name and d.tablespace_name=t.tablespace_name >>%OutputFile%
echo  and v.file#=d.file_id and t.STATUS in ('ONLINE','READ ONLY') and t.CONTENTS in ('PERMANENT','UNDO') >>%OutputFile%
echo  and (v.STATUS='ONLINE' OR v.STATUS='SYSTEM') %ReadOnlyClause% and d.tablespace_name='%TableSpaceName%';>>%OutputFile%
echo spool off;>>%OutputFile%
echo exit;>>%OutputFile%

CALL :SubRoutineExecSql
IF DEFINED CallErrorMessage 	goto :eof

CALL :SubRoutineBeginBackup
IF DEFINED CallErrorMessage 	goto :eof

SET CopyDir=%BackupDir%%GexDatabaseInstance%_%DateDir%\
MkDir %CopyDir% 2>NUL >NUL

SET InputFile=%BackupDir%hotfiles.txt
for /f "delims=" %%a in ('type "%InputFile%"') do (copy %%a %CopyDir%>NUL)&(echo Begining a hot copy of %%a.>>%LogFile%) 

CALL :SubRoutineEndBackup
IF DEFINED CallErrorMessage 	goto :eof

IF DEFINED PauseMode (echo ================PAUSE=================)&PAUSE
goto :eof

:SubRoutineHotCopyDatafiles
IF DEFINED CallErrorMessage 	goto :eof

SET InputFile=%BackupDir%tablespaces.txt
for /f "delims=" %%a in ('type "%InputFile%"') do (SET TableSpaceName=%%a)&(CALL :SubRoutineHotCopyTableSpace) 
	
IF DEFINED PauseMode (echo ================PAUSE=================)&PAUSE
goto :eof

:SubRoutineBinaryBkpCFile
IF DEFINED CallErrorMessage 	goto :eof

@echo Copy controlfiles
SET OutputFile=%BackupDir%cfbinbkp.sql
echo alter database backup controlfile to '%CopyDir%CF_%DateDir%.bkp';>%OutputFile%
echo exit>>%OutputFile%

echo Generating a binary copy of the Control File at %TIME%>>%LogFile%

CALL :SubRoutineExecSql
IF DEFINED CallErrorMessage 	goto :eof

IF DEFINED PauseMode (echo ================PAUSE=================)&PAUSE
goto :eof

:SubRoutineTraceBkpCFile
IF DEFINED CallErrorMessage 	goto :eof

@echo Copy user_dump
SET OutputFile=%BackupDir%udump.sql

	
echo set trimspool on;>%OutputFile%
echo set heading off;>>%OutputFile%
echo set feedback off;>>%OutputFile%
echo set echo off;>>%OutputFile%
echo spool %BackupDir%udump.txt>>%OutputFile%
echo select value from v$parameter where name ='user_dump_dest';>>%OutputFile%
echo exit;>>%OutputFile%

CALL :SubRoutineExecSql
IF DEFINED CallErrorMessage 	goto :eof

SET OutputFile=%BackupDir%processid.sql

echo set trimspool on;>%OutputFile%
echo set heading off;>>%OutputFile%
echo set feedback off;>>%OutputFile%
echo set echo off;>>%OutputFile%
echo spool %BackupDir%processid.txt>>%OutputFile%
echo select p.spid from v$process p, v$session s where s.paddr=p.addr and s.username=(select sys_context('USERENV','CURRENT_USER') from dual);>>%OutputFile%
echo spool off;>>%OutputFile%
echo alter database backup controlfile to trace noresetlogs;>>%OutputFile%
echo exit;>>%OutputFile%

echo Generating a trace file copy of the Control File at %DATE% - %TIME%.>>%LogFile% 

CALL :SubRoutineExecSql
IF DEFINED CallErrorMessage 	goto :eof

IF DEFINED PauseMode (echo ================PAUSE=================)&PAUSE
goto :eof

:SubRoutineLocateTrace
IF DEFINED CallErrorMessage 	goto :eof

SET InputFile=%BackupDir%udump.txt
for /f "delims=" %%a in ('type "%InputFile%"') do (SET UDump=%%a\)
SET InputFile=%BackupDir%processid.txt
for /f "delims=" %%a in ('type "%InputFile%"') do (SET ProcessId=%%a)

IF DEFINED PauseMode (echo ================PAUSE=================)&PAUSE
goto :eof

:SubRoutineProcessTraceFile
IF DEFINED CallErrorMessage 	goto :eof

SET OriginalName=%UDump%%GexDatabaseInstance%_ora_%ProcessId%.trc
SET NewName=%CopyDir%createcon%DateDir%.sql

SET Inputfile=%OriginalName%
SET OutputFile=%NewName%

echo Cleaning the trace file copy of the Control File at %TIME%>>%LogFile%
for /f "skip=24 delims=" %%a in ('type "%InputFile%"^|find /V "#"^|find /V "--"') do (echo %%a>>%OutputFile%)

IF DEFINED PauseMode (echo ================PAUSE=================)&PAUSE
goto :eof

:SubRoutineCopyControlFiles
IF DEFINED CallErrorMessage 	goto :eof

@echo Copy controlfiles ...
Set InputFile=%BackupDir%controlfiles.txt

for /f "delims=" %%a in ('type "%InputFile%"') do (copy %%a %CopyDir%>NUL)&(echo Beginning to backup controlfiles %%a.>>%LogFile%)
IF DEFINED PauseMode (echo ================PAUSE=================)&PAUSE
goto :eof

:SubRoutineListArchives
IF DEFINED CallErrorMessage 	goto :eof

SET OutputFile=%BackupDir%archivelist.sql

echo set trimspool on;>%OutputFile%
echo set heading off;>>%OutputFile%
echo set feedback off;>>%OutputFile%
echo set echo off;>>%OutputFile%
echo spool %BackupDir%archivedir.txt;>>%OutputFile%
echo SELECT MIN(SUBSTR(NAME,1,INSTR(NAME,'\',-1,1))) FROM V$ARCHIVED_LOG;>>%OutputFile%
echo spool off;>>%OutputFile%
echo spool %BackupDir%archivelist.txt;>>%OutputFile%
echo SELECT SUBSTR(NAME,INSTR(NAME,'\',-1,1)+1) FROM V$ARCHIVED_LOG;>>%OutputFile%
echo spool off;>>%OutputFile%
echo exit;>>%OutputFile%

CALL :SubRoutineExecSql
IF DEFINED CallErrorMessage 	goto :eof

IF DEFINED PauseMode (echo ================PAUSE=================)&PAUSE
goto :eof

:SubRoutineCopyArchives
IF DEFINED CallErrorMessage 	goto :eof

@echo Copy archivefiles ...

SET InputFile=%BackupDir%archivedir.txt
for /f "delims=" %%a in ('type "%InputFile%"') do SET ArchiveDir=%%a

SET InputFile=%BackupDir%archivelist.txt
for /f "delims=" %%a in ('type "%InputFile%"') do (copy "%ArchiveDir%%%a" %CopyDir%>NUL)&(echo Beginning to backup archive %%a.>>%LogFile%)

IF DEFINED PauseMode (echo ================PAUSE=================)&PAUSE
goto :eof


:SubRoutineDeleteArchives
IF DEFINED CallErrorMessage 	goto :eof

SET OutputFile=%BackupDir%delarchives.sql

echo set trimspool on;>%OutputFile%
echo set heading off;>>%OutputFile%
echo set feedback off;>>%OutputFile%
echo set echo off;>>%OutputFile%
echo spool %BackupDir%delarchives.txt;>>%OutputFile%
echo SELECT SUBSTR(NAME,INSTR(NAME,'\',-1,1)+1) from V$ARCHIVED_LOG;>>%OutputFile%
echo spool off;>>%OutputFile%
echo exit;>>%OutputFile%


CALL :SubRoutineExecSql
IF DEFINED CallErrorMessage 	goto :eof

SET InputFile=%BackupDir%archivedir.txt
for /f "delims=" %%a in ('type "%InputFile%"') do SET ArchiveDir=%%a

SET InputFile=%BackupDir%delarchives.txt
for /f "delims=" %%a in ('type "%InputFile%"') do (echo Beginning to delete original archive %%a.>>%LogFile%)&(del /F /Q "%ArchiveDir%%%a")

IF DEFINED PauseMode (echo ================PAUSE=================)&PAUSE
goto :eof

:SubRoutineCleanUp
IF DEFINED CallErrorMessage 	goto :eof

del /F /Q %BackupDir%*.sql
del /F /Q %BackupDir%*.txt

IF DEFINED PauseMode (echo ================PAUSE=================)&PAUSE
goto :eof

:Main

@echo Get Oracle Info ...
CALL :SubRoutineOracleInfo
IF DEFINED CallErrorMessage 	goto :eof
CALL :SubRoutineCheckArchivemode 
IF DEFINED CallErrorMessage 	goto :eof
CALL :SubRoutineCheckInstance
IF DEFINED CallErrorMessage 	goto :eof

IF NOT DEFINED ArchiveLogMode goto :LabelElse

@echo Begin Hot copy ...
CALL :SubRoutineLogswitch
IF DEFINED CallErrorMessage 	goto :eof
CALL :SubRoutineHotCopyDatafiles
IF DEFINED CallErrorMessage 	goto :eof
CALL :SubRoutineBinaryBkpCFile
IF DEFINED CallErrorMessage 	goto :eof
CALL :SubRoutineTraceBkpCFile
IF DEFINED CallErrorMessage 	goto :eof
CALL :SubRoutineLocateTrace
IF DEFINED CallErrorMessage 	goto :eof
CALL :SubRoutineProcessTraceFile
IF DEFINED CallErrorMessage 	goto :eof
CALL :SubRoutineLogswitch
IF DEFINED CallErrorMessage 	goto :eof
CALL :SubRoutineListArchives
IF DEFINED CallErrorMessage 	goto :eof
CALL :SubRoutineCopyArchives
IF DEFINED CallErrorMessage 	goto :eof
If DEFINED GexBackupDeleteArchives CALL :SubRoutineDeleteArchives
IF DEFINED CallErrorMessage 	goto :eof

@echo Hot Backup Complete...
echo Hot Backup Complete...>>%LogFile%
goto LabelEndIf

:LabelElse
@echo Begin Cold copy ...
CALL :SubRoutineShutInstance
CALL :SubRoutineCopyDataFiles
CALL :SubRoutineCopyControlFiles
CALL :SubRoutineStartInstance

IF DEFINED CallErrorMessage 	goto :eof
@echo Cold Backup Complete...
echo Cold Backup Complete...>>%LogFile%
:LabelEndIf

CALL :SubRoutineCleanUp 

@echo Entire backup process successfully completed at %DATE% - %TIME%.
echo Entire backup process successfully completed at %DATE% - %TIME%.>>%LogFile%

goto Exit

:Error
SET CallErrorMessage=%ErrorMessage%
IF DEFINED CallErrorMessage	@echo ERROR:%CallErrorMessage%
IF DEFINED PauseMode (echo ================PAUSE=================)&PAUSE

:Exit
REM IF DEFINED TempGalaxyFile del /q "%TempGalaxyFile%" 2>NUL
IF NOT DEFINED NoDisplay Pause