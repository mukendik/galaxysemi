@echo off
:: win32 : clone/pull "git@codebasehq.com:galaxysemi/fnp_toolkit/fnp-toolkit-win32.git" to your fnp_toolkit_directory 

SET GSACTUTIL_DIR=%1

IF "%GSACTUTIL_DIR%" == "" (
echo "-- Package dir not specified"
pause
 GOTO:EOF
)

SET FNP_TOOLKITDIR=%2
IF "%FNP_TOOLKITDIR%" == "" (
echo "-- FNP TOOLKIT DIR library not specified"
pause
 GOTO:EOF
)

IF NOT EXIST %FNP_TOOLKITDIR% (
echo "-- FNP TOOLKIT DIR does not exist"
pause
 GOTO:EOF
)

SET FNP_PLATFORM=%3
IF "%FNP_PLATFORM%" == "" (
echo "-- FNP TOOLKIT PLLATFORM not specified can be i86_n3 for win32 and x64_n6 for win64  x64_lsb for linux64 and i86_lsb for linux32"
pause
 GOTO:EOF
)

IF NOT EXIST %FNP_TOOLKITDIR%\%FNP_PLATFORM% (
echo "-- FNP TOOLKIT PLATFORM DIR does not exist"
pause
 GOTO:EOF
)



IF exist %GSACTUTIL_DIR% ( echo %GSACTUTIL_DIR% exists ) ELSE ( mkdir %GSACTUTIL_DIR% && echo %GSACTUTIL_DIR% created)
set FNP_UTILS=%GSACTUTIL_DIR%\fnp_utils
set GALAXYLM= %GSACTUTIL_DIR%\galaxylm
set LMADMIN=%GSACTUTIL_DIR%\lmadmin 

IF exist %FNP_UTILS% ( echo %FNP_UTILS% exists ) ELSE ( mkdir %FNP_UTILS% && echo %FNP_UTILS% created)
IF exist %GALAXYLM% ( echo %GALAXYLM%  exists ) ELSE ( mkdir %GALAXYLM%  && echo %GALAXYLM% created)
IF exist %LMADMIN%( echo %LMADMIN%  exists ) ELSE ( mkdir %LMADMIN%  && echo %LMADMIN% created)


copy /y %FNP_TOOLKITDIR%\%FNP_PLATFORM%\galaxylm.exe %GALAXYLM%
copy /y %FNP_TOOLKITDIR%\%FNP_PLATFORM%\galaxylm_libFNP.dll %GALAXYLM%
copy /y %FNP_TOOLKITDIR%\%FNP_PLATFORM%\galaxylm.lic %GALAXYLM%

copy /y %FNP_TOOLKITDIR%\%FNP_PLATFORM%\appactutil.exe %FNP_UTILS%
copy /y %FNP_TOOLKITDIR%\%FNP_PLATFORM%\appactutil_libFNP.dll %FNP_UTILS%
copy /y %FNP_TOOLKITDIR%\%FNP_PLATFORM%\FnpCommsSoap.dll %FNP_UTILS%
copy /y %FNP_TOOLKITDIR%\%FNP_PLATFORM%\FNP_Act_Installer.dll %FNP_UTILS%
copy /y %FNP_TOOLKITDIR%\%FNP_PLATFORM%\serveractutil.exe %FNP_UTILS%
copy /y %FNP_TOOLKITDIR%\%FNP_PLATFORM%\serveractutil_libFNP.dll %FNP_UTILS%
copy /y %FNP_TOOLKITDIR%\%FNP_PLATFORM%\tsreset_app.exe %FNP_UTILS%
copy /y %FNP_TOOLKITDIR%\%FNP_PLATFORM%\tsreset_app_libFNP.dll %FNP_UTILS%
copy /y %FNP_TOOLKITDIR%\%FNP_PLATFORM%\tsreset_svr.exe %FNP_UTILS%
copy /y %FNP_TOOLKITDIR%\%FNP_PLATFORM%\tsreset_svr_libFNP.dll %FNP_UTILS%

copy /y %FNP_TOOLKITDIR%\%FNP_PLATFORM%\lmadmin\lmadmin-i86_n3-11_11_1_1.exe %LMADMIN%

