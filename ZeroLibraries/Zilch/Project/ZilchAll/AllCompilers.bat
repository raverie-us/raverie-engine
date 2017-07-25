@echo off
call Clean.bat
REM With the MSVC/CL compiler, make sure to set your environmental variables for LIB and INCLUDE
REM These need to point to both include/lib directories in the Visual Studio/VC folder and the Windows sdk folder

@echo | call CompileMsvc2010
@echo | call CompileMsvc2013
@echo | call CompileG++
@echo | call CompileG++0x
@echo | call CompileG++Gnu0x
@echo | call CompileG++11
@echo | call CompileG++Gnu11
@echo | call CompileClang++
@echo | call CompileClang++11
@echo | call CompileEm++11

if not exist "CompileMsvc2010.exe"  ( echo CompileMsvc2010 did not compile )
if not exist "CompileMsvc2013.exe"  ( echo CompileMsvc2013 did not compile )
if not exist "CompileG++.exe"       ( echo CompileG++ did not compile )
if not exist "CompileG++0x.exe"     ( echo CompileG++0x did not compile )
if not exist "CompileG++Gnu0x.exe"  ( echo CompileG++Gnu0x did not compile )
if not exist "CompileG++11.exe"     ( echo CompileG++11 did not compile )
if not exist "CompileG++Gnu11.exe"  ( echo CompileG++Gnu11 did not compile )
if not exist "CompileClang++.exe"   ( echo CompileClang++ did not compile )
if not exist "CompileClang++11.exe" ( echo CompileClang++11 did not compile )
if not exist "CompileEm++11.html"   ( echo CompileEm++11 did not compile )
pause
