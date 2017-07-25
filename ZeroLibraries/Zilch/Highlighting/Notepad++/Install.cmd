@echo off
echo Make sure you close out any instances of Notepad++.
pause

Rem - We install a file that adds keywords to visual studio 'usertype.dat'
Rem - We also then create a file extension type for the Zilch file, and associate it with VS
cd Files
copy /Y "Zilch.xml" "%APPDATA%\Notepad++\"
ftype zilch="notepad++.exe" "%%1"
assoc .z=zilch

Rem - We're done!
cls
echo Notepad++ syntax highlighter installed!
pause
