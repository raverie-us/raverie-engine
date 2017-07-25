#define MyAppName "ZeroLauncher"
#define MyAppNameVisual "Zero Launcher"
#define MyAppPublisher "DigiPen USA Corporation"
#define MyAppURL "http://zeroengine.io"
#define MyAppExeName "ZeroLauncher.exe"

; Let these defines be defined via command line args if provided
#ifndef ZeroSource
#define ZeroSource GetEnv("ZERO_SOURCE")
#endif

#ifndef ZeroOutput
#define ZeroOutput GetEnv("ZERO_OUTPUT")
#endif

#ifndef Configuration
#define Configuration "Release"
#endif

#define ExePath "{app}\" + MyAppExeName
#define IconPath "{app}\"
                                
; TestingMode only installs the registry values and sets them to the
; ZeroOutput directory launcher (the one being built by visual studio)
;#define TestingMode

;this should be replaced via the command line arg /d{define}={value}. Also must be called with iscc.exe, not the .iss file itself.
#ifndef ZeroLauncherOutputSuffix
#define ZeroLauncherOutputSuffix "\Out\Win32\" + Configuration + "\Win32ZeroLauncher"
#endif
#ifndef MajorId
#define MajorId = 1
#endif
#ifndef MyAppVersion
;build the version id up from the major id
#define MyAppVersion MajorId + ".0.0.0"
#endif

; If we're in testing mode then change the exe output path to the built path and look at debug instead of release
#ifdef TestingMode
#define Configuration "Debug"
#define ZeroLauncherOutputSuffix "\Out\Win32\" + Configuration + "\Win32ZeroLauncher"
#define ExePath ZeroOutput + "\" + ZeroLauncherOutputSuffix + "\" + MyAppExeName
#define IconPath ZeroSource + "\Projects\Win32Shared\"
#endif

#ifndef ZeroLauncherOutputPath
#define ZeroLauncherOutputPath ZeroOutput + "\" + ZeroLauncherOutputSuffix
#endif



[Setup]
AppId={{295EE6D2-9E03-43A6-8150-388649CC1341}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
;Don't use app mutex because it doesn't work properly with silent installers
;AppMutex="ZeroLauncherMutex:{{295EE6D2-9E03-43A6-8150-388649CC1341}"
DefaultDirName={pf}\{#MyAppName}
DefaultGroupName={#MyAppName}
AllowNoIcons=yes
OutputBaseFilename=ZeroLauncherSetup
SetupIconFile="{#ZeroSource}\Projects\Win32Shared\ZeroLauncherIcon.ico"
Compression=lzma
SolidCompression=yes
WizardImageFile=ZeroInstall.bmp
ChangesEnvironment=yes
ChangesAssociations=yes
PrivilegesRequired=none
LicenseFile="{#ZeroSource}\Data\ZeroLauncherEula.txt"
CloseApplications=yes
RestartApplications=no
                                                                
[Registry] 
Root: HKCR; Subkey: ".zeroproj"; ValueType: string; ValueName: ""; ValueData: "ZeroProject"; Flags: uninsdeletevalue  noerror
Root: HKCR; Subkey: "ZeroProject"; ValueType: string; ValueName: ""; ValueData: "Zero Project"; Flags: uninsdeletekey noerror
; Keep the old zero proj icon instead of the launcher icon (the launcher icon code is commented out below)
Root: HKCR; Subkey: "ZeroProject\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{#IconPath}ZeroIcon.ico";   Flags: noerror 
;Root: HKCR; Subkey: "ZeroProject\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\{#MyAppExeName},0";   Flags: noerror 

; Add right-click shell commands to run the project with special commands
Root: HKCR; Subkey: "ZeroProject\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{#ExePath}"" ""%1"" %*";   Flags: noerror 
Root: HKCR; Subkey: "ZeroProject\shell\Open With Launcher\command"; ValueType: string; ValueName: ""; ValueData: """{#ExePath}"" ""%1"" -Upgrade";   Flags: noerror 
Root: HKCR; Subkey: "ZeroProject\shell\Run\command"; ValueType: string; ValueName: ""; ValueData: """{#ExePath}"" ""%1"" -Run";   Flags: noerror 

; Add icons for the right-click shell commands (they pull the icon from the exe)
Root: HKCR; Subkey: "ZeroProject\shell\open"; ValueType: string; ValueName: "Icon"; ValueData: """{#ExePath}""";   Flags: noerror 
Root: HKCR; Subkey: "ZeroProject\shell\Open With Launcher"; ValueType: string; ValueName: "Icon"; ValueData: """{#ExePath}""";   Flags: noerror 
Root: HKCR; Subkey: "ZeroProject\shell\Run"; ValueType: string; ValueName: "Icon"; ValueData: """{#ExePath}""";   Flags: noerror 

; Add registry keys and icons for .zerobuild files
Root: HKCR; Subkey: ".zerobuild"; ValueType: string; ValueName: ""; ValueData: "ZeroBuild"; Flags: uninsdeletevalue  noerror
Root: HKCR; Subkey: "ZeroBuild"; ValueType: string; ValueName: ""; ValueData: "Zero Build"; Flags: uninsdeletekey noerror
Root: HKCR; Subkey: "ZeroBuild\shell\open"; ValueType: string; ValueName: "Icon"; ValueData: """{#ExePath}""";   Flags: noerror 
Root: HKCR; Subkey: "ZeroBuild\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{#ExePath}"" ""%1"" %*";   Flags: noerror 
Root: HKCR; Subkey: "ZeroBuild\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{#IconPath}ZeroIcon.ico";   Flags: noerror 

; Add registry keys and icons for .zerotemplate files
Root: HKCR; Subkey: ".zerotemplate"; ValueType: string; ValueName: ""; ValueData: "ZeroTemplate"; Flags: uninsdeletevalue  noerror
Root: HKCR; Subkey: "ZeroTemplate"; ValueType: string; ValueName: ""; ValueData: "Zero Template"; Flags: uninsdeletekey noerror
Root: HKCR; Subkey: "ZeroTemplate\shell\open"; ValueType: string; ValueName: "Icon"; ValueData: """{#IconPath}ZeroTemplate.ico""";   Flags: noerror 
Root: HKCR; Subkey: "ZeroTemplate\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{#ExePath}"" ""%1"" %*";   Flags: noerror 
Root: HKCR; Subkey: "ZeroTemplate\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{#IconPath}ZeroTemplate.ico";   Flags: noerror 

; Add registry keys and icons for .zeroprojpack files
Root: HKCR; Subkey: ".zeroprojpack"; ValueType: string; ValueName: ""; ValueData: "ZeroProjPack"; Flags: uninsdeletevalue  noerror
Root: HKCR; Subkey: "ZeroProjPack"; ValueType: string; ValueName: ""; ValueData: "Zero Project Package"; Flags: uninsdeletekey noerror
Root: HKCR; Subkey: "ZeroProjPack\shell\open"; ValueType: string; ValueName: "Icon"; ValueData: """{#IconPath}ZeroPack.ico""";   Flags: noerror 
Root: HKCR; Subkey: "ZeroProjPack\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{#ExePath}"" ""%1"" %*";   Flags: noerror 
Root: HKCR; Subkey: "ZeroProjPack\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{#IconPath}ZeroPack.ico";   Flags: noerror 

; Add registry keys and icons for .zerotemplate files
Root: HKCR; Subkey: ".zeropack"; ValueType: string; ValueName: ""; ValueData: "ZeroPack"; Flags: uninsdeletevalue  noerror
Root: HKCR; Subkey: "ZeroPack"; ValueType: string; ValueName: ""; ValueData: "Zero Pack"; Flags: uninsdeletekey noerror
Root: HKCR; Subkey: "ZeroPack\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{#IconPath}ZeroPack.ico";   Flags: noerror 
                               
[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: checkablealone
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked; OnlyBelowVersion: 0,6.1

[Icons]
Name: "{group}\{#MyAppNameVisual}"; Filename: "{#ExePath}"
Name: "{group}\Uninstall {#MyAppNameVisual}"; Filename: {uninstallexe}
Name: "{commondesktop}\{#MyAppNameVisual}"; Filename: "{#ExePath}"; Tasks: desktopicon
; Add a startmenu icon to create a new project
Name: "{group}\New Zero Project"; IconFilename: "{#IconPath}ZeroLauncherIcon.ico"; Filename: "{#ExePath}"; Parameters: -New; Check: not WizardNoIcons; Tasks: desktopicon

#ifndef TestingMode
[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
;Copy over core resources
Source: "{#ZeroLauncherOutputPath}\*"; DestDir: "{app}";  Excludes: "*.exp,*.lib,*.ilib,*.ipdb,*.iobj,BuildInfo.data,__pycache__"; Flags: ignoreversion recursesubdirs createallsubdirs
;Copy all icon files over (so we can have different icons for the different shortcuts)
Source: "{#ZeroSource}\Projects\Win32Shared\*.ico"; DestDir: "{app}"; Flags: ignoreversion

[Run]
Filename: {app}\{#MyAppExeName}; Description: {cm:LaunchProgram,ZeroLauncher}; Flags: nowait postinstall skipifsilent

;Force the launcher to be closed before installing
[InstallDelete]
Type: filesandordirs; Name: "{app}\{#MyAppExeName}";
Type: filesandordirs; Name: "{localappdata}\ZeroLauncher_{#MajorId}.0";

[UninstallDelete]
Type: filesandordirs; Name: "{app}\Lib";


[Code] 

function CloseApplicationIfRunning(ApplicationName: String; ApplicationDisplayName: String; WindowName: String) : Boolean;
var
  Confirm: Integer;
  ErrorCode: Integer;
  isSilent: Boolean;
  isZeroRunning: Boolean;
begin
  Confirm := IDYES
  isSilent := WizardSilent()
  Result := True

  if isSilent = False then
  begin
    isZeroRunning := FindWindowByClassName(WindowName) <> 0
    if isZeroRunning = True then
    begin
      Confirm := MsgBox(ApplicationDisplayName + ' is already running. Do you want to close it before uninstalling?', mbConfirmation, MB_YESNO)      
    end;
  end;

  if Confirm = IDYES then                                   
    ShellExec('open', 'taskkill.exe', '/f /im ' + ApplicationName, '', SW_HIDE, ewNoWait, ErrorCode) 	  
  else
    Result := False
end;

function FindAndRemove(Root: Integer; Key: String; ApplicationName: String) : Integer;
var
  uninstaller: String;
  Confirm: Integer;
  ErrorCode: Integer;
  isSilent: Boolean;
begin
  if RegKeyExists(Root, Key) then
  begin
    Confirm := IDYES
    isSilent := WizardSilent()

    if isSilent = False then
    begin
      Confirm := MsgBox(ApplicationName + ' is already installed. Uninstall previous version?', mbConfirmation, MB_YESNO)      
    end;
	  
	  if Confirm = IDYES then
	  begin                                     
	    RegQueryStringValue(Root, Key,'UninstallString', uninstaller);
		  ShellExec('', uninstaller, '/SILENT', '', SW_HIDE, ewWaitUntilTerminated, ErrorCode); 	  
	  end;
  end;
  Result := 0;
end;

function UninstallKey(Key: String; ApplicationName: String): Boolean;
begin
    //Remove 32 bit on 32 bit
    FindAndRemove(HKEY_LOCAL_MACHINE, 'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{' + Key + '}_is1', ApplicationName);
    //Remove 32 bit on 64 bit
    FindAndRemove(HKEY_LOCAL_MACHINE, 'SOFTWARE\Wow6432Node\Microsoft\Windows\CurrentVersion\Uninstall\{' + Key + '}}_is1', ApplicationName);

    //Remove 32 bit on 32 bit
    FindAndRemove(HKEY_CURRENT_USER, 'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{' + Key + '}_is1', ApplicationName);
    //Remove 32 bit on 64 bit
    FindAndRemove(HKEY_CURRENT_USER, 'SOFTWARE\Wow6432Node\Microsoft\Windows\CurrentVersion\Uninstall\{' + Key + '}}_is1', ApplicationName);  

    Result := True;
end;
   
function InitializeSetup(): Boolean;
var RegistryKey : String;
begin
    // Legacy. Call the previous uninstaller if the launcher is installed. This shouldn't have to happen unless
    // different registry keys were installed in the past (which they were)
    RegistryKey := '295EE6D2-9E03-43A6-8150-388649CC1341';
    UninstallKey(RegistryKey, 'ZeroLauncher');

    Result := True;
end;

#endif



