SET ZeroSource=%1
SET OutDir=%2
SET FragmentExtensionsDir=%ZeroSource%\Resources\FragmentCore
SET OutputDir=%OutDir%\FragmentCore
SET FragmentSettingsDir=%ZeroSource%\Data\ZilchFragmentSettings
SET FragmentSettingsOutputDir=%OutDir%\ZilchFragmentSettings

Md %OutputDir%
Md %FragmentSettingsOutputDir%
echo %FragmentExtensionsDir%
echo %OutputDir%
copy %FragmentExtensionsDir% %OutputDir% /Y
copy %FragmentSettingsDir% %FragmentSettingsOutputDir% /Y