@echo off
echo Compilation of GTL unit test with VisualStudio at %VS80COMNTOOLS%
echo Be sure you have generated the gtl.lib with provided script in .../gtl/lib/wgx prior to running this command.

echo create_msvc2005_lib.bat:
set curdir=%CD%
cd ..\lib\wg1\
call create_msvc2005_lib.bat
cd %curdir%
echo %curdir%

if not exist ..\lib\wg1\gtl.lib (
	echo gtl.lib not found. In order to compile the unit test, please generate the MSVC GTL lib following the documentation.
	exit /b 1
)

echo Calling %VS80COMNTOOLS% initialization...
call "%VS80COMNTOOLS%\vsvars32.bat"

cl.exe main.cpp /Od /I..\include /FD /EHsc /MT /DDYNGTL /D_CRT_SECURE_NO_DEPRECATE /link ..\lib\wg1\gtl.lib /NOLOGO /SUBSYSTEM:CONSOLE
if errorlevel 1 (
   echo "Compilation failed (%errorlevel%)!"
   exit /b %errorlevel%
)

if not exist main.exe  (
    echo main.exe not found!
    exit /b 1
)

copy ..\lib\wg1\gtl.dll .
main.exe
if errorlevel 1 (
   echo "main.exe failed(%errorlevel%)!"
   exit /b %errorlevel%
)

echo Test has passed.
exit /b 0

