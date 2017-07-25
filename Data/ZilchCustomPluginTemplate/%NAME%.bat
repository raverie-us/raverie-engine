@ECHO OFF
REM If we launch Zero from Visual Studio, and then run another 'sln' from Zero it will implicitly
REM pass through all environmental variables, which we may want to clear or change
PUSHD "%~dp0"
SET VisualStudioVersion=
"%NAME%.sln"
