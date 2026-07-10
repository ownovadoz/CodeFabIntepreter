@echo off
setlocal

set "SCRIPT_DIR=%~dp0"
set "EXE=%SCRIPT_DIR%\CodeFabIntepreter.exe"

if not exist "%EXE%" (
	echo [ERROR] exe not found: %EXE%
	echo Please build Release configuration first.
	exit /b 1
)

node --version >nul 2>&1
if errorlevel 1 (
	echo [ERROR] node.exe not found on PATH. Node.js is required to generate the report.
	exit /b 1
)

node "%SCRIPT_DIR%generate_report.js"

echo.
set /p "_=Press Enter to exit..."
endlocal
