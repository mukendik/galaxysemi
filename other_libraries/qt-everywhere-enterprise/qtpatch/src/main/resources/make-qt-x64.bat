@echo off
rem -------------------------------------------------------------------------- #
rem make-qt-x64.bat
rem -------------------------------------------------------------------------- #
set qtdir=c:/cygwin/home/gexprod/qt
set qtver=5.2.1
set mingw=TDM-GCC-64-4.8.1

rem python
set path=c:/Python27;%path%

rem icu
set PATH=%PATH%;C:\icu\dist\lib
set INCLUDE=%INCLUDE%;C:\icu\dist\include
set LIB=%LIB%;C:\icu\dist\lib

rem qtwebkit
set path=c:/GnuWin32/bin;%path%
set path=c:/Perl64/bin;%path%
set path=c:/Ruby200-x64/bin;%path%
set path=c:/winFlexBison;%path%

rem mingw
set path=c:/%mingw%/bin;%path%

rem uic
set path=%path%;%qtdir%/qt-everywhere-enterprise-src-%qtver%/qtbase/lib

rem check paths
call :which python.exe
call :which icuuc.dll
call :which perl.exe
call :which ruby.exe
call :which win_flex.exe
call :which bison.exe
call :which mingw32-make.exe
call :which g++.exe

rem cd
cd %qtdir%/qt-everywhere-enterprise-src-%qtver%
if errorlevel 1 goto :eof

rem configure
if "%1" == "-no-configure" goto :make
@echo on
@echo configure...
@echo off
configure^
 -confirm-license^
 -prefix %qtdir%/%qtver%^
 -platform win32-g++^
 -release^
 -qt-zlib^
 -no-gif -qt-libpng -qt-libjpeg^
 -nomake tests -nomake examples -nomake tools^
 -skip qtwebkit-examples^
 -opengl desktop^
 -openssl^
 -developer-build^
 -I c:/%mingw%/x86_64-w64-mingw32/include -L c:/%mingw%/lib^
 -I c:/OpenSSL-Win64/include -L c:/OpenSSL-Win64/lib^
 >configure.log 2>&1

rem make
if errorlevel 1 goto :eof
:make
@echo on
@echo make...
@echo off
mingw32-make >make.log 2>&1

rem qtwebkit
if errorlevel 1 goto :eof
cd qtwebkit
@echo on
@echo make qtwebkit...
@echo off
mingw32-make qmake >make.log 2>&1
mingw32-make >>make.log 2>&1

rem install
if errorlevel 1 goto :eof
cd ..
@echo on
@echo make install...
@echo off
mingw32-make install >install.log 2>&1

rem end
goto :eof

rem which function
:which
set fullpath=%~$PATH:1
if not "%fullpath%" == "" goto :eof
@echo on
@echo %1 not found
@echo off
call :halt 1
goto :eof

rem halt function
:halt
call :__SetErrorLevel %1
call :__ErrorExit 2>nul
goto :eof

:__ErrorExit
() creates an syntax error, stops immediatly
goto :eof

:__SetErrorLevel
exit /b %time:~-2%
goto :eof

:eof
