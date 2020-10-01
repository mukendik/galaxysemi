@echo off
rem
rem This file set galaxy custom PATH
rem

REM Why do gex.exe seems to need %PROGRAMFILES%\Internet Explorer\IEShims.dll ?

set PATH=C:\Oracle\product\11.1.0\db_1\bin;C:\Oracle\product\11.1.0\db_1\OCI\include;C:\Oracle\product\11.2.0\dbhome_1\bin;C:\Oracle\product\11.2.0\dbhome_1\OCI\include;
set PATH=%PATH%;%GITDIR%\bin;
set PATH=%PATH%;%GITDIR%\cmd;
set PATH=%PATH%;%QTDIR%\5.2.1\mingw48_32\bin;
set PATH=%PATH%;%QTDIR%\Tools\mingw48_32\bin;
set PATH=%PATH%;%QTCREATORDIR%;
set PATH=%PATH%;%QTDIR%\lib;
set PATH=%PATH%;%MINGWDIR%;

rem other libs
set PATH=%PATH%;%DEVDIR%\other_libraries\chartdirector\lib\win32;
set PATH=%PATH%;%DEVDIR%\other_libraries\libqglviewer\lib\win32;
set PATH=%PATH%;%DEVDIR%\other_libraries\qtpropertybrowser-2.5_1-commercial\lib\win32;
set PATH=%PATH%;%DEVDIR%\other_libraries\database\mysql\5.1\win32;
set PATH=%PATH%;%DEVDIR%\other_libraries\quazip-0.4.4\lib\win32;
set PATH=%PATH%;%DEVDIR%\other_libraries\fnp-toolkit\lib\win32;
set PATH=%PATH%;%DEVDIR%\other_libraries\R\lib\win32;
set PATH=%PATH%;%DEVDIR%\other_libraries\tufao\0.8\lib\win32;

rem galaxy libs
set PATH=%PATH%;%DEVDIR%\galaxy_libraries\galaxy_qt_libraries\lib\win32;
set PATH=%PATH%;%DEVDIR%\galaxy_products\gex_product\gex-oc\lib\win32;
set PATH=%PATH%;%DEVDIR%\galaxy_products\gex_product\gex-pb\lib\win32;
set PATH=%PATH%;%DEVDIR%\galaxy_products\gex_product\plugins\lib\win32;
set PATH=%PATH%;%DEVDIR%\other_libraries\EProfiler\lib\win32;

rem set R_HOME
set GALAXY_R_HOME=%DEVDIR%other_libraries\R\r_home\win
rem %PROGRAMFILES%\Internet Explorer\

REM we have removed the system %PATH% from our custom PATH to be sure no user dll overides the galaxy dll pack (specially libmySQL.dll). 
REM This way, we are quite close from the client environment  

set QMAKESPEC=win32-g++
echo -- QMAKESPEC set to "win32-g++"
