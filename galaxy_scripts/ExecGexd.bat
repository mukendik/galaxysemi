@echo off

echo Setting up a Qt environment for GalaxySemi...

echo PROCESSOR_ARCHITECTURE = %PROCESSOR_ARCHITECTURE%
echo PROCESSOR_IDENTIFER = %PROCESSOR_IDENTIFIER%

:: retrieve the parent directory of the executed script in order to find out the devdir folder.
:: We cannot directly use %CD% variable as it returns the folder from where we executed the script.
:: Example: If you want to run the script as an administrator, %CD% will return the system directory and not the one of the script.
SET "DEVDIR=%~dp0"
:: for loop requires removing trailing backslash from %~dp0 output
SET "DEVDIR=%DEVDIR:~0,-1%"

SET osFolder=win32

FOR %%i IN ("%DEVDIR%") DO SET "DEVDIR=%%~dpi"


set HOMEBASEDIR=C:\Users\fclerc

set GEX_SERVER_PROFILE=%HOMEBASEDIR%\serverProfile
set CUSTOM_QTDIR=%HOMEBASEDIR%\Qt
::\home\gexprod\qt\5.6\gcc_64
set QTDIR=%CUSTOM_QTDIR%\5.2.1\5.2.1\mingw48_32
set CUSTOM_QTCREATOR=%CUSTOM_QTDIR%\qtcreator-4.6.2
set QTCREATOR=%CUSTOM_QTCREATOR%
set QTCREATORDIR=%QTCREATOR%\bin
set DEVDIR=%HOMEBASEDIR%\Work\77FromHell\galaxy_dev

set QMAKE=%QTDIR%\bin\qmake
set QTSRCDIR=%CUSTOM_QTDIR%\5.2.1\5.2.1\Src
set CUSTOM_QTSRCDIR=%QTSRCDIR%
:: export osFolder=linux64
set LC_ALL=C
echo %SHELL%
echo "GIT branch is %git_branch%"
:: export DEVDIR=%PWD/..
echo "DEVDIR set to " %DEVDIR%
set GOOGLETEST_DIR=%DEVDIR%\galaxy_tests\googletest
set GEX_PATH=%DEVDIR%\galaxy_products\gex_product\bin


::path to modify C:\Oracle\product\11.1.0\db_1\bin;C:\Oracle\product\11.1.0\db_1\OCI\include;C:\Oracle\product\11.2.0\dbhome_1\bin;C:\Oracle\product\11.2.0\dbhome_1\OCI\include;;""\bin;;""\cmd;;C:\Users\fclerc\Qt\5.2.1\5.2.1\mingw48_32\5.2.1\mingw48_32\bin;;C:\Users\fclerc\Qt\5.2.1\5.2.1\mingw48_32\Tools\mingw48_32\bin;;C:\Users\fclerc\Qt\qtcreator-4.6.2\bin;;C:\Users\fclerc\Qt\5.2.1\5.2.1\mingw48_32\lib;;;;C:\Users\fclerc\Work\V7.7\galaxy_dev\\other_libraries\chartdirector\lib\win32;;C:\Users\fclerc\Work\V7.7\galaxy_dev\\other_libraries\libqglviewer\lib\win32;;C:\Users\fclerc\Work\V7.7\galaxy_dev\\other_libraries\qtpropertybrowser-2.5_1-commercial\lib\win32;;C:\Users\fclerc\Work\V7.7\galaxy_dev\\other_libraries\database\mysql\5.1\win32;;C:\Users\fclerc\Work\V7.7\galaxy_dev\\other_libraries\quazip-0.4.4\lib\win32;;C:\Users\fclerc\Work\V7.7\galaxy_dev\\other_libraries\fnp-toolkit\lib\win32;;C:\Users\fclerc\Work\V7.7\galaxy_dev\\other_libraries\R\lib\win32;;C:\Users\fclerc\Work\V7.7\galaxy_dev\\other_libraries\tufao\0.8\lib\win32;;C:\Users\fclerc\Work\V7.7\galaxy_dev\\galaxy_libraries\galaxy_qt_libraries\lib\win32;;C:\Users\fclerc\Work\V7.7\galaxy_dev\\galaxy_products\gex_product\gex-oc\lib\win32;;C:\Users\fclerc\Work\V7.7\galaxy_dev\\galaxy_products\gex_product\gex-pb\lib\win32;;C:\Users\fclerc\Work\V7.7\galaxy_dev\\galaxy_products\gex_product\plugins\lib\win32;;C:\Users\fclerc\Work\V7.7\galaxy_dev\\other_libraries\EProfiler\lib\win32;

echo osFolder = %osFolder%
echo system = %system%

::  LIBRARY_PATH=\
::  %QTDIR/bin:\
::  %QTDIR/lib:\
::  %{DEVDIR}/galaxy_libraries/galaxy_std_libraries/lib/%osFolder:\
::  %{DEVDIR}/galaxy_libraries/galaxy_qt_libraries/lib/%osFolder:\
::  %{DEVDIR}/other_libraries/libqglviewer/lib/%osFolder:\
::  %{DEVDIR}/galaxy_products/gex_product/gex-pb/lib/%osFolder:\
::  %{DEVDIR}/galaxy_products/gex_product/gex-oc/lib/%osFolder:\
::  %{DEVDIR}/galaxy_products/gex_product/gex-log/lib/%osFolder:\
::  %{DEVDIR}/galaxy_products/gex_product/plugins/lib/%osFolder:\
::  %{DEVDIR}/other_libraries/qtpropertybrowser-2.5_1-commercial/lib/%osFolder:\
::  %{DEVDIR}/other_libraries/R/lib/%osFolder
::  %{DEVDIR}/other_libraries/chartdirector/lib/linux64:\
::  %{DEVDIR}/other_libraries/fnp-toolkit/lib/%osFolder:\
::  %{DEVDIR}/galaxy_products/gex_product/bin/lib
::  
::  set R_HOME


set LANG=en
set LD_LIBRARY_PATH=%LIBRARY_PATH%
echo "LIBRARY_PATH set to " %LD_LIBRARY_PATH%

echo "QTCREATORDIR set to " %QTCREATORDIR%
REM echo "PATH set to " %PATH%
echo "QTDIR = " %QTDIR%
echo "QTSRCDIR = " %QTSRCDIR%
echo "QTCREATOR_DIR = " %CUSTOM_QTCREATOR%
echo "GALAXY_R_HOME set to " %GALAXY_R_HOME%
echo osFolder = %osFolder%
echo system = %system%
echo "Launching QtCreator from " %CUSTOM_QTCREATOR%

:: export DISPLAY=:0

IF "%QTDIR%" == "" (
echo "-- No supported Qt libraries found in C:\ ! You must install Qt libraries package (C:\Qt\Qtx.y.z) in order to work with this repository."
pause
exit
)
IF "%QTCREATORDIR%" == "" (
echo "-- Qt Creator not found !"
pause
exit
)

Rem set QMAKESPEC=win32-g++
Rem echo -- QMAKESPEC set to "win32-g++"


:: REM set QTSRCDIR=%QTDIR%\src
echo -- DEVDIR set to %DEVDIR%
echo -- QTDIR set to %QTDIR%
echo -- QTSRCDIR set to %QTSRCDIR%
echo -- QTCREATORDIR set to %QTCREATORDIR%

echo -- Setting QT_HASH_SEED to 0
set QT_HASH_SEED=0

set GITDIR=""
IF exist "%PROGRAMFILES%\Git" set GITDIR="%PROGRAMFILES%\Git"
if exist "%PROGRAMFILES(X86)%\Git" set GITDIR="%PROGRAMFILES(X86)%\Git"
IF exist C:\Git set GITDIR=C:\Git
IF %GITDIR% == ""  echo -- Warning : git program not found ! git should not work inside QtCreator !
echo -- GITDIR set to %GITDIR% 
IF %GITDIR% NEQ "" %GITDIR%\bin\git.exe --version 
echo --

call %DEVDIR%/galaxy_scripts/SetGalaxyPath


set PATH=%DEVDIR%\galaxy_libraries\galaxy_std_libraries\lib\%osFolder%;%PATH%;
set PATH=%DEVDIR%\other_libraries\libqglviewer\lib\%osFolder%;%PATH%;
set PATH=%DEVDIR%\galaxy_products\gex_product\gex-log\lib\%osFolder%;%PATH%;
set PATH=%DEVDIR%\other_libraries\chartdirector\lib\%osFolder%;%PATH%;
set PATH=%DEVDIR%\galaxy_products\gex_product\bin\lib;%PATH%;
set GALAXY_R_HOME=%DEVDIR%\other_libraries\R\r_home\%osFolder%;
set PATH=%DEVDIR%\galaxy_products\gex_product\plugins\lib\%osFolder%;%PATH%;
set PATH=%DEVDIR%\galaxy_products\gex_product\bin;%PATH%;

echo --
qmake -v
echo --

set LANG=en
echo -- LANG set to %LANG%
echo -- path

echo --
g++ --version
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

REM %QTCREATORDIR%\qtcreator.exe -help
REM %QTCREATORDIR%\qtcreator.exe -version
REM pause

REM launching something..
start /B %QTCREATORDIR%\qtcreator.exe
::start /B %DEVDIR%\galaxy_unit_tests\build-galaxy_unit_test_5-Qt5_6_2_MSVC_14-Debug\debug\galaxy_unit_test_5.exe
::start /B %HOMEBASEDIR%\Downloads\deleteme\depends.exe %DEVDIR%\galaxy_unit_tests\build-galaxy_unit_test_5-Qt5_6_2_MSVC_14-Debug\debug\galaxy_unit_test_5.exe
REM pause

pause
