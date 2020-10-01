echo off
echo Compilation of unit test with VisualStudio
echo Be sure you have generated the gtl.lib with provided script in .../gtl/lib/wgx prior to running this command.
echo Be also sure to set the correct VC variables with such command :
echo "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" x86_amd64

REM Static linking does not seem to work anymore from 7.2 :
Rem cl.exe main.cpp /I..\include /MD /link ..\lib\wg2\libgtl_core.a ..\lib\wg2\libgstdl.a ws2_32.lib

cl.exe main.cpp /I..\include /DDYNGTL /link ..\lib\wg2\gtl.lib
pause
