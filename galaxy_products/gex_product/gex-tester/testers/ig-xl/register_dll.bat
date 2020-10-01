echo off

REM Let's register in all different possible ways, just in case, as dll registration seems painfull on Windows.

copy libgcc_s_dw2-1.dll c:\Windows\System32
copy gtl-igxl.dll c:\Windows\System32
copy mingwm10.dll c:\Windows\System32

REM libgcc_s_dw2-1.dll has no DllRegisterServer nor DllInstall but let s try to register anyway even if unusefull
REM Let's register silently (/s) to avoid unusefull warning message about DllRegisterServer
c:\Windows\System32\REGSVR32.exe /s c:\Windows\System32\libgcc_s_dw2-1.dll
c:\Windows\System32\REGSVR32.exe /s c:\Windows\System32\mingwm10.dll
REM gtl-igxl.dll has now a DllRegisterServer function which should be called
c:\Windows\System32\REGSVR32.exe c:\Windows\System32\gtl-igxl.dll

REM 64b system : to be tested :
if exist c:\Windows\SysWOW64\REGSVR32.exe (
	echo Copying and registering dll in SysWOW64 ...
	copy libgcc_s_dw2-1.dll c:\Windows\SysWOW64
	copy gtl-igxl.dll c:\Windows\SysWOW64
	copy mingwm10.dll c:\Windows\SysWOW64

	c:\Windows\SysWOW64\REGSVR32.exe /s c:\Windows\SysWOW64\mingwm10.dll
	c:\Windows\SysWOW64\REGSVR32.exe /s c:\Windows\SysWOW64\libgcc_s_dw2-1.dll
	c:\Windows\SysWOW64\REGSVR32.exe c:\Windows\SysWOW64\gtl-igxl.dll
)
pause
