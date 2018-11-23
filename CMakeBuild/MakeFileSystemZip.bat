REM Make sure 7-Zip is installed and added to the path
cd ..

REM Make the editor file system
del /F /Q "BuildOutput/ZeroEditorFileSystem.zip"
7z a -tzip -mx=9 -mfb=128 -mpass=10 "BuildOutput/ZeroEditorFileSystem.zip" "Resources/Loading" "Resources/FragmentCore" "Resources/ZeroCore" "Resources/Editor" "Resources/EditorUi" "Resources/UiWidget" "PrebuiltZeroContent" "Data/ColorSchemes" "Data/Fallback" "Data/Licenses" "Data/ZilchFragmentSettings" "Data/*.*"

REM Make the launcher file system
del /F /Q "BuildOutput/ZeroLauncherFileSystem.zip"
7z a -tzip -mx=9 -mfb=128 -mpass=10 "BuildOutput/ZeroLauncherFileSystem.zip" "Resources/Loading" "Resources/FragmentCore" "Resources/ZeroCore" "Resources/ZeroLauncherResources" "PrebuiltZeroContent" "Data/ColorSchemes" "Data/Fallback" "Data/Licenses" "Data/ZilchFragmentSettings" "Data/*.*"

pause