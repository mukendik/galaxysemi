@echo off

echo Setting up a 32bits Qt environment for GalaxySemi...

echo PROCESSOR_ARCHITECTURE = %PROCESSOR_ARCHITECTURE%
echo PROCESSOR_IDENTIFER = %PROCESSOR_IDENTIFIER%

subst S: C:\Work

:: retrieve the parent directory of the executed script in order to find out the devdir folder.
:: We cannot directly use %CD% variable as it returns the folder from where we executed the script.
:: Example: If you want to run the script as an administrator, %CD% will return the system directory and not the one of the script.
SET "DEVDIR=%~dp0"
:: for loop requires removing trailing backslash from %~dp0 output
SET "DEVDIR=%DEVDIR:~0,-1%"

FOR %%i IN ("%DEVDIR%") DO SET "DEVDIR=%%~dpi"

echo -- DEVDIR set to %DEVDIR%

echo Searching for QTDIR : must be the folder where we can find subfolders include, lib, mkspecs...

REM IF exist C:\Mingw						set MINGWDIR=C:\Mingw\bin

REM IF exist C:\Qt\QtSDK (
rem set QTDIR=C:\Qt\QtSDK\Desktop\Qt\4.7.2\mingw
rem set QTCREATORDIR=C:\Qt\QtSDK\QtCreator\bin
rem set MINGWDIR=C:\Qt\QtSDK\mingw\bin
rem set QTSRCDIR=C:\Qt\QtSDK\QtSources\4.7.2\src
rem )

rem IF exist C:\QtSDK (
rem set QTDIR=C:\QtSDK\Desktop\Qt\4.7.2\mingw
rem set QTCREATORDIR=C:\QtSDK\QtCreator\bin
rem set MINGWDIR=C:\QtSDK\mingw\bin
rem set QTSRCDIR=C:\QtSDK\QtSources\4.7.2\src
rem )

rem IF exist C:\Qt\QtSDK (
rem set QTSDKDIR=C:\Qt\QtSDK
rem )
rem IF exist C:\QtSDK (
rem set QTSDKDIR=C:\QtSDK
rem )

rem IF "%QTSDKDIR%" == "" (
rem echo "-- No supported Qt SDK found in C:\ ! You must install QtSDK (C:\Qt\QtSDK or C:\QtSDK) package in order to work with this repository."
rem pause
rem exit
rem )

IF 	exist C:\Qt\Qt5.2.1 (
 set QTDIR=C:\Qt\Qt5.2.1\
 set QTCREATORDIR=C:\Qt\Qt5.2.1\Tools\QtCreator\bin
 set MINGWDIR=C:\Qt\Qt5.2.1\Tools\mingw48_32\mingw\bin
 set QTSRCDIR=C:\Qt\Qt5.2.1\5.2.1\Src
)

REM Check if commercial version installed in C:\Qt\Qt5.2.1_Digia
REM Due to strange Windows behavior, use_digia is initialized and checked outside a IF block (doesn't work otherwise)
set use_digia=n
IF 	exist C:\Qt\Qt5.2.1_Digia (
 set /p use_digia=If you want to use Qt from C:\Qt\Qt5.2.1_Digia, type "y": 
)
IF "%use_digia%"=="y" (
  set QTDIR=C:\Qt\Qt5.2.1_Digia\
  set QTCREATORDIR=C:\Qt\Qt5.2.1_Digia\Tools\QtCreator\bin
  set MINGWDIR=C:\Qt\Qt5.2.1_Digia\Tools\mingw48_32\mingw\bin
  set QTSRCDIR=C:\Qt\Qt5.2.1_Digia\5.2.1\Src
)

REM set QTSRCDIR=%QTDIR%\src
echo -- QTSRCDIR set to %QTSRCDIR%

IF "%QTDIR%" == "" (
echo "-- No supported Qt Qt5.2.1 libraries found in C:\ ! You must install Qt Qt5.2.1 libraries package (C:\Qt\Qt5.2.1) in order to work with this repository."
pause
exit
)

echo -- Setting QT_HASH_SEED to 0
set QT_HASH_SEED=0

IF "%QTCREATORDIR%" == "" (
echo "-- Qt Creator not found !"
pause
exit
)

IF "%MINGWDIR%" == "" (
echo "-- MinGW not found !"
pause
exit
)

echo -- QTDIR set to %QTDIR%
echo -- QTCREATORDIR set to %QTCREATORDIR%
echo -- MINGWDIR set to %MINGWDIR%

set GITDIR=""
IF exist "%PROGRAMFILES%\Git" set GITDIR="%PROGRAMFILES%\Git"
if exist "%PROGRAMFILES(X86)%\Git" set GITDIR="%PROGRAMFILES(X86)%\Git"
IF exist C:\Git set GITDIR=C:\Git
IF %GITDIR% == ""  echo -- Warning : git program not found ! git should not work inside QtCreator !
echo -- GITDIR set to %GITDIR% 
IF %GITDIR% NEQ "" %GITDIR%\bin\git.exe --version 
echo --

call %DEVDIR%/galaxy_scripts/SetGalaxyPath

echo --
qmake -v
echo --

set LANG=en
echo -- LANG set to %LANG%
echo --
path

echo --
echo According to DevConfig.ods, this branch must be build with GCC 4.8.0. 
g++ --version
echo -- gdb is currently version 7.5.1 with Qt5.2.1 win32...
gdb -v

echo --
echo GS_QA = %GS_QA%
echo GEX_SERVER_PROFILE = %GEX_SERVER_PROFILE%
echo GEX_LOGLEVEL = %GEX_LOGLEVEL%
echo GEXLP_LOGLEVEL = %GEXLP_LOGLEVEL%
echo GEXOC_LOGLEVEL = %GEXOC_LOGLEVEL%
echo GEXPB_LOGLEVEL = %GEXPB_LOGLEVEL%
echo GEXDB_PLUGIN_GALAXY_LOGLEVEL = %GEXDB_PLUGIN_GALAXY_LOGLEVEL%
echo GEXDB_PLUGIN_BASE_LOGLEVEL = %GEXDB_PLUGIN_BASE_LOGLEVEL%
echo GALAXY_R_HOME = %GALAXY_R_HOME%
echo --
echo Launching QtCreator in %QTCREATORDIR% and opening session master...

REM %QTCREATORDIR%\qtcreator.exe -help
REM %QTCREATORDIR%\qtcreator.exe -version
REM pause

REM Bernard: please comment if you change something
IF "%USERNAME%" == "will" ( start /B %QTCREATORDIR%\qtcreator.exe master ) else ( start /B %QTCREATORDIR%\qtcreator.exe )

pause
Quit
