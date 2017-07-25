@echo off
REM On Windows XP LOCALAPPDATA is not defined CSIDL_LOCAL_APPDATA will redirect to %USERPROFILE%\Local Settings\Application Data
IF DEFINED LOCALAPPDATA (ECHO Has Local App Data) ELSE (SET LOCALAPPDATA=%USERPROFILE%\Local Settings\Application Data)
REM Clean all content data
rmdir /s /q "%LOCALAPPDATA%\ZeroContent"