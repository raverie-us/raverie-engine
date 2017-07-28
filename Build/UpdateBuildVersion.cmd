echo off
SET ZERO_SOURCE=%~1
SET BuildFolder=%ZERO_SOURCE%\Build
SET BuildVersionIdsFolder=%BuildFolder%\BuildVersionIds
SET TempFile=%BuildFolder%\BuildVersion.temp

REM Check if mercurial exists
call hg status > nul
if %errorlevel% == 0 goto :MercurialPresent

REM otherwise, check if git exists
call git status > nul
if %errorlevel% == 0 goto :GitPresent

REM otherwise, fallback on having no source control
goto :NoSourceControl

REM Run the mercurial commit information extraction step
:MercurialPresent
call UpdateBuildVersion.hg.cmd %1 %TempFile%
goto :Finish

REM Run the git commit information extraction step
:GitPresent
call UpdateBuildVersion.git.cmd %1 %TempFile%
goto :Finish

REM Run a backup information extraction step (uses stub info)
:NoSourceControl
call UpdateBuildVersion.none.cmd %1 %TempFile%
goto :Finish

:Finish
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
