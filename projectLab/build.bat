@echo off
REM ===========================================================
REM  Smart Home Console - G9 - classroom build
REM  Author: Ahmed Ellamie <ahmed.ellamiee@gmail.com>
REM ===========================================================
setlocal

cd /d "%~dp0"

set "SRC=main.c src\house.c src\render.c src\ui.c src\platform.c src\demo.c"

gcc -std=c99 -Wall -Wextra -I"%~dp0include" -o "%~dp0house.exe" %SRC%
if errorlevel 1 goto fail

echo Build OK. Run: house.exe
goto :eof

:fail
echo BUILD FAILED - check the errors above.
exit /b %errorlevel%
