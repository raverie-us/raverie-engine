@ECHO OFF
REM Used to automatically build the plugin in Release
PUSHD "%~dp0"
SET VisualStudioVersion=
"%VS140COMNTOOLS%/../IDE/devenv" "RESOURCE_NAME_.sln" /build Release
