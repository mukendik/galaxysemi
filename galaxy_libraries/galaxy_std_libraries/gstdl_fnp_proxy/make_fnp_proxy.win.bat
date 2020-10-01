:: usage pass the destination library path the fnp toolkit dir and the fnp platforme type
:: make_fnp_proxy.win.bat DESTLIB_DIR FNP_TOOLKITDIR FNP_PLATFORM
:: PLATFORM can be i86_n3 for win32 and x64_n6 for win64  x64_lsb for linux64 and i86_lsb for linux32
@echo off

SET DESTLIB_DIR=%1
IF "%DESTLIB_DIR%" == "" (
    echo "-- Destination library not specified"
    exit /B 1
)

IF NOT EXIST %DESTLIB_DIR% (
    echo "-- Destination library does not exist"
    exit /B 1
)

SET FNP_TOOLKITDIR=%2
IF "%FNP_TOOLKITDIR%" == "" (
    echo "-- FNP TOOLKIT DIR library not specified"
    exit /B 1
)

IF NOT EXIST %FNP_TOOLKITDIR% (
    echo "-- FNP TOOLKIT DIR does not exist"
    exit /B 1
)

SET FNP_PLATFORM=%3
IF "%FNP_PLATFORM%" == "" (
    echo "-- FNP TOOLKIT PLLATFORM not specified can be i86_n3 for win32 and x64_n6 for win64  x64_lsb for linux64 and i86_lsb for linux32"
    exit /B 1
)

IF NOT EXIST %FNP_TOOLKITDIR%\%FNP_PLATFORM% (
    echo "-- FNP TOOLKIT PLATFORM DIR does not exist"
    exit /B 1
)

set PATH=%FNP_TOOLKITDIR%\%FNP_PLATFORM%;%PATH%

SET CURRENT_DIR=%CD%
cd  /D %CURRENT_DIR%

set MAKEFILE_ACT=make_fnp_proxy.act.%FNP_PLATFORM%
IF NOT EXIST %MAKEFILE_ACT% (
    echo "-- Makefile to build the toolkit not found must be make_fnp_proxy.act.%FNP_PLATFORM%"
    exit /B 1
)

IF NOT EXIST %CURRENT_DIR%\release\win32 (
   mkdir %CURRENT_DIR%\release\win32
)

nmake -f %MAKEFILE_ACT% FNP_PROVIDER_DIR_M=%CURRENT_DIR% FNP_TOOLKITDIR_M=%FNP_TOOLKITDIR% DESTLIB_DIR_M=%DESTLIB_DIR% FNP_PLATFORM_M=%FNP_PLATFORM%
SET ERRORLEV=%ERRORLEVEL%
IF %ERRORLEV% NEQ 0 (
    echo "make command failure (%ERRORLEV%)"
    exit /B %ERRORLEV%
)

copy /y %CURRENT_DIR%\sources\fnp_proxy.xml %FNP_TOOLKITDIR%\%FNP_PLATFORM%

cd  /D %FNP_TOOLKITDIR%\%FNP_PLATFORM%
preptool.exe -v fnp_proxy.xml
if errorlevel 1 (
    echo "preptool command failure (%errorlevel%)"
    exit /B 1
)

@echo on
copy /y fnp.dll %DESTLIB_DIR%
copy /y fnp_libFNP.dll %DESTLIB_DIR%
copy /y FNP_Act_Installer.dll %DESTLIB_DIR%

cd  /D %CURRENT_DIR%
