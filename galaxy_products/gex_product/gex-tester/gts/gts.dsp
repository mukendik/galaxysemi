# Microsoft Developer Studio Project File - Name="gts" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=gts - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "gts.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "gts.mak" CFG="gts - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "gts - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "gts - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/gex-tester/gts", ABGAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "gts - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD CPP /nologo /MD /W3 /O1 /I "$(QTDIR)\include" /I "D:\Dev\gex_product\gex-tester\gts" /I "C:\Qt\3.3.2\mkspecs\win32-msvc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "QT_DLL" /D "QT_THREAD_SUPPORT" /D "QT_NO_DEBUG" /FD -Zm200 /c
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /machine:IX86
# ADD LINK32 "qt-mt332.lib" "qtmain.lib" "kernel32.lib" "user32.lib" "gdi32.lib" "comdlg32.lib" "advapi32.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" "imm32.lib" "winmm.lib" "wsock32.lib" "winspool.lib" "kernel32.lib" "user32.lib" "gdi32.lib" "comdlg32.lib" "advapi32.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" "imm32.lib" "winmm.lib" "wsock32.lib" "winspool.lib" "opengl32.lib" "glu32.lib" "delayimp.lib" delayimp.lib gtl_core.lib gstdutils_c.lib gerrormgr.lib /nologo /subsystem:windows /machine:IX86 /libpath:"$(QTDIR)\lib" /DELAYLOAD:comdlg32.dll /DELAYLOAD:oleaut32.dll /DELAYLOAD:winmm.dll /DELAYLOAD:wsock32.dll /DELAYLOAD:winspool.dll

!ELSEIF  "$(CFG)" == "gts - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD CPP /nologo /MDd /W3 /Zi /Od /I "$(QTDIR)\include" /I "D:\Dev\gex_product\gex-tester\gts" /I "C:\Qt\3.3.2\mkspecs\win32-msvc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "QT_DLL" /D "QT_THREAD_SUPPORT" /FD /GZ -Zm200 /c
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /machine:IX86
# ADD LINK32 "qt-mt332.lib" "qtmain.lib" "kernel32.lib" "user32.lib" "gdi32.lib" "comdlg32.lib" "advapi32.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" "imm32.lib" "winmm.lib" "wsock32.lib" "winspool.lib" "kernel32.lib" "user32.lib" "gdi32.lib" "comdlg32.lib" "advapi32.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" "imm32.lib" "winmm.lib" "wsock32.lib" "winspool.lib" "opengl32.lib" "glu32.lib" "delayimp.lib" gtl_core_debug.lib gstdutils_c_debug.lib gerrormgr_debug.lib /nologo /subsystem:windows /debug /machine:IX86 /pdbtype:sept /libpath:"$(QTDIR)\lib"

!ENDIF 

# Begin Target

# Name "gts - Win32 Release"
# Name "gts - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\gts_mainwindow.cpp
# End Source File
# Begin Source File

SOURCE=main.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\gts_mainwindow.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "Forms"

# PROP Default_Filter "ui"
# Begin Source File

SOURCE=gts_mainwindow_base.ui

!IF  "$(CFG)" == "gts - Win32 Release"

USERDEP__GTS_M="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.
InputPath=gts_mainwindow_base.ui
InputName=gts_mainwindow_base

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(InputDir)\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\$(InputName).cpp \
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp \
	

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "gts - Win32 Debug"

USERDEP__GTS_M="$(QTDIR)\bin\moc.exe"	"$(QTDIR)\bin\uic.exe"	
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.
InputPath=gts_mainwindow_base.ui
InputName=gts_mainwindow_base

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(InputDir)\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\$(InputName).cpp \
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp \
	

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Group "Images"

# PROP Default_Filter "png jpeg bmp xpm"
# Begin Source File

SOURCE=images/editcopy
# End Source File
# Begin Source File

SOURCE=images/editcut
# End Source File
# Begin Source File

SOURCE=images/editpaste
# End Source File
# Begin Source File

SOURCE=images/filenew

!IF  "$(CFG)" == "gts - Win32 Release"

USERDEP__FILEN="images/filenew"	"images/fileopen"	"images/filesave"	"images/print"	"images/undo"	"images/redo"	"images/editcut"	"images/editcopy"	"images/editpaste"	"images/searchfind"	
# Begin Custom Build - Creating image collection...
InputPath=images/filenew

"qmake_image_collection.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\uic -embed gts -f images.tmp -o qmake_image_collection.cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "gts - Win32 Debug"

USERDEP__FILEN="images/filenew"	"images/fileopen"	"images/filesave"	"images/print"	"images/undo"	"images/redo"	"images/editcut"	"images/editcopy"	"images/editpaste"	"images/searchfind"	
# Begin Custom Build - Creating image collection...
InputPath=images/filenew

"qmake_image_collection.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\uic -embed gts -f images.tmp -o qmake_image_collection.cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=images/fileopen
# End Source File
# Begin Source File

SOURCE=images/filesave
# End Source File
# Begin Source File

SOURCE=images/print
# End Source File
# Begin Source File

SOURCE=images/redo
# End Source File
# Begin Source File

SOURCE=images/searchfind
# End Source File
# Begin Source File

SOURCE=images/undo
# End Source File
# End Group
# Begin Group "Generated"

# PROP Default_Filter ""
# Begin Source File

SOURCE=gts_mainwindow_base.cpp
# End Source File
# Begin Source File

SOURCE=gts_mainwindow_base.h
# End Source File
# Begin Source File

SOURCE=moc_gts_mainwindow_base.cpp
# End Source File
# Begin Source File

SOURCE=qmake_image_collection.cpp
# End Source File
# End Group
# Begin Group "GEX sources"

# PROP Default_Filter "*.cpp; *.h"
# Begin Source File

SOURCE=..\..\gex\cstdf.cpp
# End Source File
# Begin Source File

SOURCE=..\..\gex\cstdf.h
# End Source File
# Begin Source File

SOURCE=..\..\gex\cstdfparse_v4.cpp
# End Source File
# Begin Source File

SOURCE=..\..\gex\cstdfparse_v4.h
# End Source File
# Begin Source File

SOURCE=..\..\gex\cstdfrecords_v4.cpp
# End Source File
# Begin Source File

SOURCE=..\..\gex\cstdfrecords_v4.h
# End Source File
# End Group
# End Target
# End Project
