echo off
SET ZERO_SOURCE=%~1
SET TempFile=%~2
SET BuildFolder=%ZERO_SOURCE%\Build
SET BuildVersionIdsFolder=%BuildFolder%\BuildVersionIds

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
