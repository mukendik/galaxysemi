# Microsoft Developer Studio Project File - Name="DPAT_Testing2" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=DPAT_Testing2 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "DPAT_Testing2.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "DPAT_Testing2.mak" CFG="DPAT_Testing2 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "DPAT_Testing2 - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe
# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Debug"
# PROP Intermediate_Dir ".\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /Zp1 /MTd /W3 /GX /ZI /Od /I "C:\asl_nt\system\include" /I "c:\ASL_NT\Users\Lists\DPAT_Testing2" /I "C:\Max_nt\galaxy\include" /D "_DEBUG" /D "WIN32" /D "_MT" /D "_DLL" /D "_WINDOWS" /D "_MBCS" /D "NT" /D "USER_CODE" /D "TEST_EXE" /Yu"asl.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 StatusReport.lib CommonShared.lib smartvariable.lib WFType.lib sitemanager.lib utilities.lib test.lib board.lib develop.lib screen.lib meter.lib handler.lib devices.lib msvcrtd.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib UserCode.lib pc416dll.lib pc416drv.obj gpib-32.obj /nologo /entry:"_DllMainCRTStartup@12" /subsystem:windows /dll /debug /machine:I386 /nodefaultlib:"libc.lib" /nodefaultlib:"libcmt.lib" /nodefaultlib:"msvcrt.lib" /nodefaultlib:"libcd.lib" /nodefaultlib:"libcmtd.lib" /libpath:"C:\asl_nt\system\library"
# SUBTRACT LINK32 /map
# Begin Target

# Name "DPAT_Testing2 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "*.cpp"
# Begin Source File

SOURCE=.\DynamicPAT.cpp
# End Source File
# Begin Source File

SOURCE=.\Fake_Test.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Max_nt\galaxy\src\gtl_tester_asl.cpp
# End Source File
# Begin Source File

SOURCE=.\NEW_FUNC.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "*.h"
# Begin Source File

SOURCE=.\DynamicPAT.h
# End Source File
# Begin Source File

SOURCE=.\Fake_Test.h
# End Source File
# Begin Source File

SOURCE=.\NEW_FUNC.h
# End Source File
# End Group
# Begin Group "System Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\BRD_CFG.CPP
# End Source File
# Begin Source File

SOURCE=.\DLLMAIN.CPP
# End Source File
# Begin Source File

SOURCE=.\FUNCTION.H
# End Source File
# Begin Source File

SOURCE=.\LIST.CPP
# End Source File
# Begin Source File

SOURCE=.\LIST.H
# End Source File
# Begin Source File

SOURCE=.\REGISTER.CPP
# End Source File
# Begin Source File

SOURCE=.\USER.CPP
# ADD CPP /Yc"asl.h"
# End Source File
# Begin Source File

SOURCE=.\USER.H
# End Source File
# End Group
# End Target
# End Project
