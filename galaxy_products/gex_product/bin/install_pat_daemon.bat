@echo off
rem -------------------------------------------------------------------------- #
rem install_pat_daemon.bat
rem -------------------------------------------------------------------------- #
set service=QX-PAT-Man

gs-pmd.exe -i

set valuename=ImagePath

set keyname=HKLM\SYSTEM\CurrentControlSet\services\%service%
reg query %keyname% /v %valuename%
if errorlevel 0 goto :editkey

set keyname=HKLM\SYSTEM\ControlSet002\services\%service%
reg query %keyname% /v %valuename%
if errorlevel 0 goto :editkey

set keyname=HKLM\SYSTEM\ControlSet001\services\%service%
reg query %keyname% /v %valuename%
if errorlevel 0 goto :editkey

@echo on
@echo error: %keyname%\%valuename% not found
@echo off
goto :eof

:editkey
for /f "tokens=2*" %%a in ('reg query %keyname% /v %valuename% ^|^
 find "%valuename%"') do set value=%%b
set value="%value% -platform minimal"
reg add %keyname% /v %valuename% /d %value% /f

:eof
