@ECHO OFF
REM Cleans all the various Visual Studio artefacts that can get left behind
REM Note: This also cleans up specific user settings to a project
PUSHD "%~dp0"
DEL /F /S /Q /AH "*.suo"
DEL /F /S /Q "*.user"
DEL /F /S /Q "*.sdf"
DEL /F /S /Q "*.opensdf"
DEL /F /S /Q "*.sdf"
DEL /F /S /Q "*.db"
DEL /F /S /Q "*.opendb"
RD /S /Q "Debug"
RD /S /Q "Release"
RD /S /Q ".vs"
RD /S /Q "ipch"
RD /S /Q "bin"
RD /S /Q "obj"
CLS
ECHO All Visual Studio Artifacts Removed
