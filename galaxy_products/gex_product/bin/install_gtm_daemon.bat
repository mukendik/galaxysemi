@echo off

echo Checking for admin rights...
net session >nul 2>&1
if %errorLevel% == 0 (
        echo Success: Administrative permissions confirmed.
) else (
        echo Failure: Current permissions inadequate. Installation will quit.
		pause
		exit
)

REM whoami is not available on all winXP...
if exist %windir%\system32\whoami.exe (
        echo Checking whoami...
        whoami /groups | find "S-1-16-12288" > nul
        if not errorlevel 1 (
                echo Success: This user seems to be admin.
        ) else (
                echo Failure: Not an admin user. Installation will quit.
                pause
                exit
        )
) else ( echo Note: whoami executable not found... )

echo Checking server profile...
REM if "%GTM_SERVER_PROFILE%" == "" (
IF not DEFINED GTM_SERVER_PROFILE (
	echo GTM_SERVER_PROFILE not found. This environment variable is mandatory in order to run the GTM daemon. 
	echo Please set it to a writable folder. 
	echo Please also be sure to run at least once the GTM GUI using this server profile in order to set up licensing.
	pause
	exit
) else (
	echo Success: GTM_SERVER_PROFILE found : %GTM_SERVER_PROFILE%
)
pause

echo Installing the GTM daemon...
REM Be sure to be in the installation folder...
SET mypath=%~dp0
cd %mypath:~0,-1%
gs-gtmd.exe -i
if %ERRORLEVEL% GEQ 1 (
	echo Failed to install the dameon : error %ERRORLEVEL%. Check the logs for root cause.
	pause
	exit
)
echo Starting service...

net start "GS-GTM"

pause
