@ECHO OFF
REM Installs the Visual Studio plugin and then runs our solution
PUSHD "%~dp0"
ZeroZilchPlugins.vsix
DEL /F /Q ZeroZilchPlugins.vsix
"%NAME%.bat"
