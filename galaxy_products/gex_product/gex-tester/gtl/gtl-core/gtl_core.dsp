# Microsoft Developer Studio Project File - Name="gtl_core" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=gtl_core - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "gtl_core.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "gtl_core.mak" CFG="gtl_core - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "gtl_core - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "gtl_core - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/gex-tester/gtl/gtl_core", PAGAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "gtl_core - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy ".\Release\gtl_core.lib" "..\lib\gtl_core.lib"
# End Special Build Tool

!ELSEIF  "$(CFG)" == "gtl_core - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy ".\Debug\gtl_core.lib" "..\lib_debug\gtl_core_debug.lib"
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "gtl_core - Win32 Release"
# Name "gtl_core - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\gtc\gtc_netmessage.c
# End Source File
# Begin Source File

SOURCE=.\gtl_error.c
# End Source File
# Begin Source File

SOURCE=.\gtl_main.c
# End Source File
# Begin Source File

SOURCE=.\gtl_message.c
# End Source File
# Begin Source File

SOURCE=.\gtl_server.c
# End Source File
# Begin Source File

SOURCE=.\gtl_socket.c
# End Source File
# Begin Source File

SOURCE=.\gtl_testlist.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\gtc\gtc_constants.h
# End Source File
# Begin Source File

SOURCE=..\..\gtc\gtc_netmessage.h
# End Source File
# Begin Source File

SOURCE=.\gtl_constants.h
# End Source File
# Begin Source File

SOURCE=.\gtl_core_image.h
# End Source File
# Begin Source File

SOURCE=.\gtl_core_simul.h
# End Source File
# Begin Source File

SOURCE=.\gtl_error.h
# End Source File
# Begin Source File

SOURCE=.\gtl_main.h
# End Source File
# Begin Source File

SOURCE=.\gtl_message.h
# End Source File
# Begin Source File

SOURCE=.\gtl_server.h
# End Source File
# Begin Source File

SOURCE=.\gtl_socket.h
# End Source File
# Begin Source File

SOURCE=.\gtl_testlist.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\gtl_tester.conf
# End Source File
# End Target
# End Project
