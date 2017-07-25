echo off
SET ZERO_SOURCE=%~1
SET BuildFolder=%ZERO_SOURCE%\Build
SET BuildVersionIdsFolder=%BuildFolder%\BuildVersionIds
SET TempFile=%BuildFolder%\BuildVersion.temp

REM Determine if there's a mercurial repository here, if not then fall back on a base implementation
call hg status > nul
if NOT %errorlevel% == 0 goto :NoMercurialPresent

REM Always add a pragma once to the top of this file
echo #pragma once > %TempFile%

REM Capture the branch name
SET cmd=call hg log -q -r ^"max(parents())^" --template ^"{branch}^"
FOR /F "tokens=*" %%i IN (' %cmd% ') DO SET BranchName=%%i

REM If the build id file for this branch doesn't exist then define the
REM experimental branch name as the current branch and use the backup build id file
SET BuildIdBranchFileName="%BuildVersionIdsFolder%\%BranchName%.txt"

IF NOT EXIST %BuildIdBranchFileName% (
  echo Build id file did not exist, using backup
  echo #define ZeroExperimentalBranchName "%BranchName%" >> %TempFile%
  SET BuildIdBranchFileName="%BuildVersionIdsFolder%\Backup.txt"
)

echo Branch Id Filename is %BuildIdBranchFileName%

REM Always write out the build id file contents
type %BuildIdBranchFileName% >> %TempFile%

REM Write out the various identifiers from the current revision's node (Branch, Changset, Revision, etc...)
call hg log -q -r "max(parents())" --style "%BuildFolder%\BuildVersion.style" >> %TempFile%
goto :TryCopyTempFile

REM Mercurial wasn't present so just copy the backup file and some default values for the changeset
:NoMercurialPresent
echo No mercurial present, copying backup file with default values

echo #pragma once > %TempFile%
type %BuildVersionIdsFolder%\Backup.txt >> %TempFile%
echo #define ZeroRevisionId 0 >> %TempFile%
echo #define ZeroShortChangeSet 0 >> %TempFile%
echo #define ZeroChangeSet 0 >> %TempFile%
echo #define ZeroChangeSetDate "" >> %TempFile%
echo #define ZeroBranchName "" >> %TempFile%
goto :TryCopyTempFile


:TryCopyTempFile
REM Only copy this file over the one in source if it's different or doesn't exist.
REM Do this to prevent always rebuilding.
fc %TempFile% "%ZERO_SOURCE%\Systems\Engine\BuildVersion.inl" > nul
if %errorlevel%==0 (
  echo BuildVersion file was the same, not updating to avoid rebuild.
  goto :removetemp
)
copy %TempFile% "%ZERO_SOURCE%\Systems\Engine\BuildVersion.inl"

REM Delete the temporary file
:removetemp
del %TempFile%
