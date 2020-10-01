@echo off
rem
rem This file set galaxy custom PATH
rem

REM Why do gex.exe seems to need %PROGRAMFILES%\Internet Explorer\IEShims.dll ?

set PATHBKP=%PATH%
set PATH=%GITDIR%\bin;%GITDIR%\cmd;%QTDIR%\bin;%QTCREATORDIR%;%QTDIR%\lib;%MINGWDIR%;%DEVDIR%\other_libraries\chartdirector\lib\win32;%DEVDIR%\other_libraries\libqglviewer\lib\win32;%DEVDIR%\other_libraries\qtpropertybrowser-2.5_1-commercial\lib\win32;%DEVDIR%\other_libraries\database\mysql\5.1\win32;%DEVDIR%\other_libraries\quazip-0.4.4\lib\win32;%DEVDIR%\galaxy_libraries\galaxy_qt_libraries\lib\win32;%DEVDIR%\galaxy_products\gex_product\gex-oc\lib\win32;%DEVDIR%\galaxy_products\gex_product\gex-pb\lib\win32;%DEVDIR%\galaxy_products\gex_product\gex-log\lib\win32;%DEVDIR%\galaxy_products\gex_product\plugins\lib\win32;%PATHBKP%


REM we have removed the system %PATH% from our custom PATH to be sure no user dll overides the galaxy dll pack (specially libmySQL.dll). 
REM This way, we are quite close from the client environment  

set QMAKESPEC=win32-g++
echo -- QMAKESPEC set to "win32-g++"
