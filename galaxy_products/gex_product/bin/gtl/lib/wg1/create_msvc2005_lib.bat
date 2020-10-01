@echo off
REM LIB portal: http://msdn.microsoft.com/en-us/library/7ykb2k5f.aspx
REM Using LIB to create import file: http://msdn.microsoft.com/en-us/library/f0z8kac4.aspx
REM Building an Import Library: http://msdn.microsoft.com/en-us/library/0b9xe492.aspx
REM If you are accustomed to the LINK32.exe and LIB32.exe tools provided with the Microsoft Win32 Software Development Kit for Windows NT, 
REM you may have been using either the command link32 -lib or the command lib32 for managing libraries and creating import libraries. 
REM Be sure to change your makefiles and batch files to use the lib command instead.

echo This script allow to create a static Microsoft VisualStudio library (.lib) file from the glt .def file and the dll 
echo It MUST be run from the Visual Studio command prompt only. 
call "%VS80COMNTOOLS%\vsvars32.bat"
if errorlevel 1 (
   echo "call %VS80COMNTOOLS%\vsvars32.bat failed(%errorlevel%)!"
   exit /b %errorlevel%
)

lib.exe /machine:i386 /def:gtl.def /VERBOSE
if errorlevel 1 (
   echo "lib.exe /machine:i386 /def:gtl.def /VERBOSE failed(%errorlevel%)!"
   exit /b %errorlevel%
)
echo gtl lib created
