SET ZeroSource=%1
SET OutDir=%2
SET FragmentExtensionsDir=%ZeroSource%\Resources\FragmentCore
SET OutputDir=%OutDir%\FragmentCore

SET FragmentExtensionsIRDir=%ZeroSource%\UnitTests\ZilchShaders\FragmentExtensions
SET FragmentExtensionsIROutputDir=%OutDir%\FragmentExtensionsIR

SET FragmentSettingsDir=%ZeroSource%\Data\ZilchFragmentSettings
SET FragmentSettingsOutputDir=%OutDir%\ZilchFragmentSettings

Md %OutputDir%
Md %FragmentExtensionsIROutputDir%
Md %FragmentSettingsOutputDir%
echo %FragmentExtensionsDir%
echo %FragmentExtensionsIRDir%
echo %OutputDir%
copy %FragmentExtensionsDir% %OutputDir% /Y
xcopy %FragmentExtensionsIRDir% %FragmentExtensionsIROutputDir% /Y /S
copy %FragmentSettingsDir% %FragmentSettingsOutputDir% /Y