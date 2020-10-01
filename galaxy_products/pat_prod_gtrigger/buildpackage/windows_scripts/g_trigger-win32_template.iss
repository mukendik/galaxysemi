; G-Trigger

[Setup]
; Application Details
AppName=G-Trigger
AppVerName=Galaxy G-Trigger V{%VERSION}
AppCopyright=(c) 2000-2015. All rights reserved
OutputBaseFilename=g_trigger_v{%VERSION}-{%DISTGROUP}
OutputDir=..\packages
; Version details
AppVersion={%AppVersion}
VersionInfoVersion={%VersionInfoVersion}
;
AppPublisher=Galaxy
AppPublisherURL=http://www.galaxysemi.com
AppSupportURL=http://www.galaxysemi.com
AppUpdatesURL=http://www.galaxysemi.com
DefaultDirName={pf}\Galaxy G-Trigger
DefaultGroupName=Galaxy G-Trigger
WizardImageFile=wizard.bmp
SetupIconFile=../../sources/gtrigger.ico
UninstallIconFile=../../sources/gtrigger.ico
WizardImageStretch=no
Compression=lzma
SolidCompression=yes
ChangesAssociations=yes
; Disable to confirm the group name to create
DisableProgramGroupPage=yes
; Disable to display 'ready to install'
DisableReadyPage=yes
; Icon to be used in the Windows Add/Remove programs list
UninstallDisplayIcon={app}\gtrigger.ico
;password
; Show the Select Destination Location wizard page (auto/yes/no).
; See http://www.jrsoftware.org/ishelp/index.php?topic=setup_disabledirpage
DisableDirPage=no
DisableWelcomePage=no

[LangOptions]
WelcomeFontName=Verdana
WelcomeFontSize=10

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}";

[Files]
; GEX product
Source: "../..\bin\g-trigger.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "../..\sources\gtrigger.ico"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\common_files\readme.txt"; DestDir: "{app}"; Flags: ignoreversion
; GS libraries
Source: "..\..\..\..\galaxy_libraries\galaxy_qt_libraries\lib\win32\gqtl_patcore.dll"; DestDir: "{app}"; Flags: ignoreversion
; QT5 dlls and drivers
Source: "{%QTDIR}\bin\Qt5Core.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{%QTDIR}\bin\Qt5Gui.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{%QTDIR}\bin\Qt5Widgets.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{%QTDIR}\bin\Qt5Xml.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{%ICU_PATH}\icuin52.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{%ICU_PATH}\icuuc52.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{%ICU_PATH}\icudt52.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{%MINGWDIR}\bin\libwinpthread-1.dll"; DestDir: "{app}"; Flags: ignoreversion
; STDC++
Source: "{%MINGWDIR}\bin\libstdc++-6.dll"; DestDir: "{app}"; Flags: ignoreversion
; GCC 4.8 dll
Source: "{%MINGWDIR}\bin\libgcc_s_dw2-1.dll"; DestDir: "{app}"; Flags: ignoreversion
; Qt5 packages need to include qwindows.dll
Source: "{%QTDIR}\plugins\platforms\qwindows.dll"; DestDir: "{app}\platforms"; Flags: ignoreversion
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
; Links
Name: "{group}\G-Trigger"; Filename: "{app}\g-trigger.exe"; WorkingDir: "{app}"; IconFilename: "{app}\gtrigger.ico"

; Desktop shortcut
Name: "{userdesktop}\Galaxy G-Trigger"; Filename: "{app}\g-trigger"; Tasks: desktopicon; WorkingDir: "{app}"; IconFilename: "{app}\gtrigger.ico"

[Run]
Filename: "{app}\g-trigger.exe"; Description: "{cm:LaunchProgram,Galaxy G-Trigger}"; Flags: nowait postinstall skipifsilent
