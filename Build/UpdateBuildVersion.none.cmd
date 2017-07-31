echo off
SET ZERO_SOURCE=%~1
SET TempFile=%~2
SET BuildFolder=%ZERO_SOURCE%\Build
SET BuildVersionIdsFolder=%BuildFolder%\BuildVersionIds

echo #pragma once > %TempFile%
REM Always read from the default version id file
type %BuildVersionIdsFolder%\default.txt >> %TempFile%

REM We don't have any source control so just hard-code all of this information to empty
echo #define ZeroRevisionId 0 >> %TempFile%
echo #define ZeroShortChangeSet 0 >> %TempFile%
echo #define ZeroChangeSet 0 >> %TempFile%
echo #define ZeroChangeSetDate "" >> %TempFile%
