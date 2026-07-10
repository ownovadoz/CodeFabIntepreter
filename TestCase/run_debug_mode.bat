@echo off
setlocal enabledelayedexpansion

set "SCRIPT_DIR=%~dp0"
set "EXE=%SCRIPT_DIR%..\CodeFabIntepreter\x64\Release\CodeFabIntepreter.exe"

if not exist "%EXE%" (
	echo [ERROR] exe not found: %EXE%
	echo Please build Release configuration first.
	exit /b 1
)

echo ===== 12_debug_mode_demo.txt ^(debug mode^) =====
"%EXE%" debug "%SCRIPT_DIR%12_debug_mode_demo.txt"
echo.

echo All test cases finished.
endlocal
