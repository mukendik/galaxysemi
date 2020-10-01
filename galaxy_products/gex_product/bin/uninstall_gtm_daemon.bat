@echo off
echo Stoping the daemon...
net stop "GS-GTM"

echo Uninstalling the GTM daemon...
gs-gtmd.exe -u
if %ERRORLEVEL% GEQ 1 echo Failed to uninstall the dameon : error %ERRORLEVEL%. Check the logs for root cause.
pause

REM echo Suppress service ?
REM sc delete GS-GTM