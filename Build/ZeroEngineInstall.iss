#define MyAppName "ZeroEditor"
#define MyAppPublisher "DigiPen Institute of Technology"
#define MyAppURL "http://zero.digipen.edu/"
#define MyAppExeName "ZeroEditor.exe"

#ifndef ZeroSource
#define ZeroSource GetEnv("ZERO_SOURCE")
#endif

#ifndef ZeroOutput
#define ZeroOutput GetEnv("ZERO_OUTPUT")
#endif

;this should be replaced via the command line arg /d{define}={value}. Also must be called with iscc.exe, not the .iss file itself.
#ifndef ZeroEditorOutputSuffix
#define ZeroEditorOutputSuffix "\Out\Win32\Release\ZeroEditor"
#endif
#ifndef MyAppVersion
#define MyAppVersion "0.0.1.5238"
#endif


[Setup]
AppId={{F6E3D203-EB81-4B09-983C-31DAD32AE29F}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={pf}\{#MyAppName}
DefaultGroupName={#MyAppName}
AllowNoIcons=yes
OutputBaseFilename=ZeroEngineSetup
SetupIconFile="{#ZeroSource}\Projects\Win32Shared\ZeroIcon.ico"
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
Source: "{#ZeroSource}\Resources\*"; DestDir: "{app}\Resources"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#ZeroSource}\Tools\*"; Excludes: "*.hg"; DestDir: "{app}\Tools"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#ZeroOutput}\{#ZeroEditorOutputSuffix}\*"; DestDir: "{app}";  Excludes: "*.exp,*.lib,*.ilib,*.ipdb,*.iobj,*.ilk,BuildInfo.data,__pycache__"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#ZeroSource}\Data\*"; DestDir: "{app}\Data"; Flags: ignoreversion recursesubdirs createallsubdirs
;Source: "ZeroDoc.chm"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

;Copy over redist
;Source: "vcredist_x86_2010.exe"; DestDir: {tmp}; Flags: ignoreversion
;Source: "dxwebsetup.exe"; DestDir: {tmp}; Flags: ignoreversion

[UninstallDelete]
Type: filesandordirs; Name: "{app}\Lib";

[Registry] 
Root: HKCR; Subkey: ".zeroproj"; ValueType: string; ValueName: ""; ValueData: "ZeroProject"; Flags: uninsdeletevalue  noerror
Root: HKCR; Subkey: "ZeroProject"; ValueType: string; ValueName: ""; ValueData: "Zero Project"; Flags: uninsdeletekey noerror
Root: HKCR; Subkey: "ZeroProject\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\{#MyAppExeName},0";   Flags: noerror 
Root: HKCR; Subkey: "ZeroProject\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#MyAppExeName}"" ""%1""";   Flags: noerror 

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
;Name: "{group}\Zero Documentation"; Filename: "{app}\ZeroDoc.chm"
Name: "{group}\Uninstall Zero Editor"; Filename: {uninstallexe}
Name: "{commondesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\Tools\CleanZeroContent.cmd"; Description: "Clean Content Cache(Highly Recommended)"; Flags: postinstall runhidden

;Filename: {tmp}\vcredist_x86_2010.exe; Parameters: /q; StatusMsg: Installing Visual C++ 2010 Redistributable...  
;Filename: {tmp}\dxwebsetup.exe; Parameters: /q; StatusMsg: Installing DirectX...

[Code] 

function CloseCurrentZero() : Boolean;
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
    isZeroRunning := FindWindowByClassName('ZeroWindow') <> 0
    if isZeroRunning = True then
    begin
      Confirm := MsgBox('Zero Engine is already running. Do you want to close it before uninstalling?', mbConfirmation, MB_YESNO)      
    end;
  end;

  if Confirm = IDYES then                                    
    ShellExec('open', 'taskkill.exe', '/f /im ZeroEditor.exe', '', SW_HIDE, ewNoWait, ErrorCode) 	  
  else
    Result := False
end;

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
      Confirm := MsgBox('Zero Engine is already installed. Uninstall previous version?', mbConfirmation, MB_YESNO)      
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
    zeroIsNotRunning := CloseCurrentZero()
    if zeroIsNotRunning = False then
      Result := False
    else
    begin
      //Remove 32 bit on 32 bit
      FindAndRemove(HKEY_LOCAL_MACHINE, 'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{F6E3D203-EB81-4B09-983C-31DAD32AE29F}_is1');
      //Remove 32 bit on 64 bit
      FindAndRemove(HKEY_LOCAL_MACHINE, 'SOFTWARE\Wow6432Node\Microsoft\Windows\CurrentVersion\Uninstall\{F6E3D203-EB81-4B09-983C-31DAD32AE29F}}_is1');

      //Remove 32 bit on 32 bit
      FindAndRemove(HKEY_CURRENT_USER, 'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{F6E3D203-EB81-4B09-983C-31DAD32AE29F}_is1');
      //Remove 32 bit on 64 bit
      FindAndRemove(HKEY_CURRENT_USER, 'SOFTWARE\Wow6432Node\Microsoft\Windows\CurrentVersion\Uninstall\{F6E3D203-EB81-4B09-983C-31DAD32AE29F}}_is1');  

      Result := True;
    end;
end;



