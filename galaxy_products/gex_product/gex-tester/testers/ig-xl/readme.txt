GalaxySemi GTL-IGXL library version 3.2
linked with GTL core 3.2
July 11th 2013

Purpose : 
This library is needed in order for a Microsoft Excel/VisualBasic 2003 (i.e. IG-XL) test program to interact with the GTL solution of GalaxySemi.

Tested & validated with : Microsoft Excel 2003 32bit running on Microsoft Windows 7 (32 or 64 bit).

Installation:
- copy paste the attached dll in Windows/System32 and, if any, Windows/SysWOW64

- register the dll : use regsvr32.exe to register the dll : 
	- Warning : on 64bits Windows, there are 2 regsvr32.exe : one in System32 and another in SysWOW64. 
	- On 64bits: if the system32 regsvr32 does not work, try the one in SysWOW64. 
	- Note: The Windows dialog appearing when running regsvr32.exe could warn about "DllRegisterServer" not found but this is actually working on Win7. The latest version of GTL-IGXL now contains this function and regsvr32 should n't complain anymore about that function. 
	On trouble to register the dll toward Microsoft Windows OS, please follow such above links and/or contact Microsoft support:
	http://www.microsoft.com/resources/documentation/windows/xp/all/proddocs/en-us/regsvr32.mspx?mfr=true
	http://support.microsoft.com/kb/249873
	On success, a gtl-igxl-register.txt file should appear close to the dll.

- Once dll registered, please open and follow instructions in gtl-igxl_unit_test.xls in order to run the unit test (does not require the IG-XL environnement).

- in case of undesired behavior, please check:
	- the gtl-igxl....txt log file generally generated in C:\Program Files (x86)\Teradyne\IG-XL\5.10.55_flx\bin
	- the syslog messages emited by both the GTL and GTM through any syslog listener/server (as for example StarSyslogDaemon, SyslogWatcher, ...)

	
 Copyright Galaxy
 This computer program is protected by copyright law
 and international treaties. Unauthorized reproduction or
 distribution of this program, or any portion of it,may
 result in severe civil and criminal penalties, and will be
 prosecuted to the maximum extent possible under the law.
