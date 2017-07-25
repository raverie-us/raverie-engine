SET OutDir=%1
SET SourceDir=%OutDir%\Win32ZeroLauncherDll
SET DestDir=%OutDir%\Win32ZeroLauncher

Md %DestDir%
copy %SourceDir%\ZeroLauncherDll.dll %DestDir% /Y
copy %SourceDir%\ZeroLauncherDll.pdb %DestDir% /Y
