SET OutDir=%1
SET SourceDir=%OutDir%\ZeroLauncherSharedLibrary
SET DestDir=%OutDir%\ZeroLauncher

Md %DestDir%
copy %SourceDir%\ZeroLauncherSharedLibrary.dll %DestDir% /Y
copy %SourceDir%\ZeroLauncherSharedLibrary.pdb %DestDir% /Y
