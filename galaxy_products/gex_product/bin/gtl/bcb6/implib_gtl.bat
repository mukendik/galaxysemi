REM This script is going to use Borland 'implib' tool in order to create a GTL BorlandCBuilder compatible library in order to use the GTL dyn lib with a BCB executable. 
REM Using option -a : Add '_' alias for MS flavor cdecl functions
REM On success, a libgtl.lib file should be created in order to link your Borland C Builder program with
copy ..\lib\win32\gtl.dll .

"%ProgramFiles(x86)%\Borland\CBuilder6\Bin\implib.exe" -a libgtl.lib gtl.dll

REM Copying some dll in order for the unit test program to work
copy ..\lib\win32\mingwm10.dll .
pause
