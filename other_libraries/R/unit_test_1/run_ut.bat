echo setting DEVDIR...
set DEVDIR=../../..
REM set PATH=C:\TDM-GCC-64-4.8.1\bin;C:\cygwin\home\gexprod\qt\5.2.1\lib;C:\icu\dist\lib;
echo Setting PATH...
set PATH=^
c:\Qt\Qt5.2.1\Tools\mingw48_32\bin\;^
c:\Qt\Qt5.2.1\5.2.1\mingw48_32\bin\;^
c:\Qt\Qt5.2.1\5.2.1\mingw48_32\lib\;^
..\lib\win32
set R_HOME=..\r_home\win
set GALAXY_R_HOME=%R_HOME%
set R_LIBS=..\r_home\win\library

echo %PATH%
release\unit_test_1
pause
