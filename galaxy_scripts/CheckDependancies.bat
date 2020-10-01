@echo off

SET DEVDIR=%CD%\..
echo -- DEVDIR set to %DEVDIR%

IF exist C:\Qt\QtSDK (
set QTDIR=C:\Qt\QtSDK\Desktop\Qt\4.8.1\mingw
set QTCREATORDIR=C:\Qt\QtSDK\QtCreator\bin
set MINGWDIR=C:\Qt\QtSDK\mingw\bin
set QTSRCDIR=C:\Qt\QtSDK\QtSources\4.8.1\src
)

IF exist C:\QtSDK (
set QTDIR=C:\QtSDK\Desktop\Qt\4.8.1\mingw
set QTCREATORDIR=C:\QtSDK\QtCreator\bin
set MINGWDIR=C:\QtSDK\mingw\bin
set QTSRCDIR=C:\QtSDK\QtSources\4.8.1\src
)

IF 	exist C:\Qt\Qt5.1.1 (
 set QTDIR=C:\Qt\Qt5.1.1\
 set QTCREATORDIR=C:\Qt\Qt5.1.1\Tools\QtCreator\bin
 set MINGWDIR=C:\Qt\Qt5.1.1\Tools\mingw48_32\mingw\bin
 set QTSRCDIR=C:\Qt\Qt5.1.1\5.1.1\Src
)

IF 	exist C:\Qt\Qt5.2.1 (
 set QTDIR=C:\Qt\Qt5.2.1\
 set QTCREATORDIR=C:\Qt\Qt5.2.1\Tools\QtCreator\bin
 set MINGWDIR=C:\Qt\Qt5.2.1\Tools\mingw48_32\mingw\bin
 set QTSRCDIR=C:\Qt\Qt5.2.1\5.2.1\Src
)

IF "%QTDIR%" == "" echo -- No supported Qt SDK / QtCreator found in C:\Qt !
IF "%QTDIR%" == "" pause
IF "%QTDIR%" == "" exit

IF "%QTCREATORDIR%" == "" (
echo -- Qt Creator directory not found !
pause
exit
)

IF "%MINGWDIR%" == "" (
echo -- MinGW directory not found !
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

call %DEVDIR%/galaxy_scripts/SetGalaxyPath

set LANG=en
echo -- LANG set to %LANG%

set QTSRCDIR=%QTDIR%\src
echo -- QTSRCDIR set to %QTSRCDIR%

path

IF exist C:\depends22_x86 (
	echo dependencywalker detected : searching for gex.exe...

	pause

	IF exist %DEVDIR%\galaxy_products\gex_product\bin\gexd.exe (
		echo gexd exec found : checking dependancies of gexd.exe...
		pause
		rem call /wait
		C:\depends22_x86\depends.exe /c /ot %HOMEPATH%\GalaxySemi\gex_dependancies.txt /oc %HOMEPATH%\GalaxySemi\gex_dependancies.csv %DEVDIR%/galaxy_products/gex_product/bin/gexd.exe
		echo depends.exe returned %errorlevel% %ERRORLEVEL%
		pause
		IF errorlevel  1 echo Error %errorlevel% : one or more modules of gex.exe has not been found ! List of unfindable modules in gex_dependancies.csv/txt 
		IF errorlevel  1 call C:\depends22_x86\depends.exe  %DEVDIR%/galaxy_products/gex_product/bin/gexd.exe
	)
)

pause 

if exist "%PROGRAMFILES(X86)%\PE Explorer" (
	echo running PE explorer
	pause
	"%PROGRAMFILES(X86)%\PE Explorer\pexplorer.exe" %DEVDIR%/galaxy_products/gex_product/bin/gexd.exe
)

pause