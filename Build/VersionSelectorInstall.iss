#define MyAppName "ZeroVersionSelector"
#define MyAppPublisher "DigiPen Institute of Technology"
#define MyAppURL "http://zero.digipen.edu/"
#define MyAppExeName "ZeroVersionSelector.exe"
#define ZeroSource GetEnv("ZERO_SOURCE")
#define ZeroOutput GetEnv("ZERO_OUTPUT")

;this should be replaced via the command line arg /d{define}={value}. Also must be called with iscc.exe, not the .iss file itself.
#ifndef ZeroVersionSelectorOutputSuffix
#define ZeroVersionSelectorOutputSuffix "\Out\Win32\Release\ZeroVersionSelector"
#endif
#ifndef MyAppVersion
#define MyAppVersion "0.0.1.0"                                         
#endif


[Setup]
AppId={{295EE6D2-9E03-43A6-8150-388649CC1341}                  
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={pf}\{#MyAppName}
DefaultGroupName={#MyAppName}
AllowNoIcons=yes
OutputBaseFilename=ZeroVersionSelectorSetup
SetupIconFile="{#ZeroSource}\Projects\Win32Shared\ZeroVersionSelectorIcon.ico"
Compression=lzma
SolidCompression=yes
WizardImageFile=ZeroInstall.bmp
ChangesEnvironment=yes
ChangesAssociations=yes
PrivilegesRequired=none
LicenseFile=ZeroEngineEula.txt

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked; OnlyBelowVersion: 0,6.1

[Files]
;Copy over core resources
Source: "{#ZeroSource}\Resources\VersionSelector\*"; DestDir: "{app}\Resources\VersionSelector"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#ZeroSource}\Resources\Loading\*"; DestDir: "{app}\Resources\Loading"; Flags: ignoreversion recursesubdirs createallsubdirs
;Source: "{#ZeroSource}\Resources\*"; DestDir: "{app}\Resources"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#ZeroSource}\Tools\*"; Excludes: "*.hg"; DestDir: "{app}\Tools"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#ZeroOutput}\{#ZeroVersionSelectorOutputSuffix}\*"; DestDir: "{app}";  Excludes: "*.exp,*.lib,BuildInfo.data,__pycache__"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#ZeroSource}\Data\*"; DestDir: "{app}\Data"; Flags: ignoreversion recursesubdirs createallsubdirs
;Source: "ZeroDoc.chm"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[UninstallDelete]
Type: filesandordirs; Name: "{app}\Lib";

[Registry] 
Root: HKCR; Subkey: ".zeroproj"; ValueType: string; ValueName: ""; ValueData: "ZeroProject"; Flags: uninsdeletevalue  noerror
Root: HKCR; Subkey: "ZeroProject"; ValueType: string; ValueName: ""; ValueData: "Zero Project"; Flags: uninsdeletekey noerror
 
Root: HKCR; Subkey: "ZeroProject\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#MyAppExeName}"" ""%1""";   Flags: noerror 
Root: HKCR; Subkey: "ZeroProject\shell\upgrade\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#MyAppExeName}"" ""%1"" -upgrade";   Flags: noerror 
Root: HKCR; Subkey: "ZeroProject\shell\run\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#MyAppExeName}"" ""%1"" -run";   Flags: noerror 
;Root: HKCR; Subkey: "ZeroProject\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """C:\BuildOutput\Out\Win32\Debug\ZeroVersionSelector\ZeroVersionSelector.exe"" ""%1""";   Flags: noerror 
;Root: HKCR; Subkey: "ZeroProject\shell\upgrade\command"; ValueType: string; ValueName: ""; ValueData: """C:\BuildOutput\Out\Win32\Debug\ZeroVersionSelector\ZeroVersionSelector.exe"" ""%1"" -upgrade";   Flags: noerror 
;Root: HKCR; Subkey: "ZeroProject\shell\run\command"; ValueType: string; ValueName: ""; ValueData: """C:\BuildOutput\Out\Win32\Debug\ZeroVersionSelector\ZeroVersionSelector.exe"" ""%1"" -run";   Flags: noerror 

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
;Name: "{group}\Zero Documentation"; Filename: "{app}\ZeroDoc.chm"
Name: "{group}\Uninstall Zero Version Selector"; Filename: {uninstallexe}
Name: "{commondesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon


[Code] 


function FindAndRemove(Root: Integer; Key: String) : Integer;
var
  uninstaller: String;
  Confirm: Integer;
  ErrorCode: Integer;
  isSilent: Boolean;
  isZeroRunning: Boolean;
begin
  if RegKeyExists(Root, Key) then
  begin
    Confirm := IDYES
    isSilent := WizardSilent()

    if isSilent = False then
    begin
      Confirm := MsgBox('Zero Version Selector is already installed. Uninstall previous version?', mbConfirmation, MB_YESNO)      
    end;
	  
	  if Confirm = IDYES then
	  begin                                     
	    RegQueryStringValue(Root, Key,'UninstallString', uninstaller);
		  ShellExec('', uninstaller, '/SILENT', '', SW_HIDE, ewWaitUntilTerminated, ErrorCode); 	  
	  end;
  end;
end;
   
function InitializeSetup(): Boolean;
var
  zeroIsNotRunning : Boolean;
begin
    //Remove 32 bit on 32 bit
    FindAndRemove(HKEY_LOCAL_MACHINE, 'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{295EE6D2-9E03-43A6-8150-388649CC1341}_is1');
    //Remove 32 bit on 64 bit
    FindAndRemove(HKEY_LOCAL_MACHINE, 'SOFTWARE\Wow6432Node\Microsoft\Windows\CurrentVersion\Uninstall\{295EE6D2-9E03-43A6-8150-388649CC1341}}_is1');

    //Remove 32 bit on 32 bit
    FindAndRemove(HKEY_CURRENT_USER, 'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{295EE6D2-9E03-43A6-8150-388649CC1341}_is1');
    //Remove 32 bit on 64 bit
    FindAndRemove(HKEY_CURRENT_USER, 'SOFTWARE\Wow6432Node\Microsoft\Windows\CurrentVersion\Uninstall\{295EE6D2-9E03-43A6-8150-388649CC1341}}_is1');  

    Result := True;
end;




