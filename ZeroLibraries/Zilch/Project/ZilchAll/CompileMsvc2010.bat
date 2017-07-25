@echo off
cls
echo ***********************************************************************
echo ****************************** MSVC 2010 ******************************
echo ***********************************************************************
call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\bin\vcvars32"
"C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\bin\cl" Zilch.cpp main.cpp /W4 /Fe%~n0.exe > %~n0.log 2>&1
type %~n0.log
echo ****************************** COMPLETED ******************************
pause
