@echo off
setlocal enabledelayedexpansion

:: FieldTrackerApp - Debug start (with console output)
set "APP_DIR=%~dp0"
set "VENV_DIR=%APP_DIR%.venv"
set "PYTHON_EXE=%VENV_DIR%\Scripts\python.exe"

echo =========================================
echo  FieldTrackerApp - DEBUG MODUS
:: Use python.exe (not pythonw.exe) so console shows errors

echo.
echo App Ordner: %APP_DIR%
echo.

if not exist "%APP_DIR%requirements.txt" (
    echo FEHLER: requirements.txt nicht gefunden.
    pause
    exit /b 1
)

if not exist "%VENV_DIR%" (
    echo .venv fehlt. Erstelle zuerst mit START.bat.
    pause
    exit /b 1
)

echo Starte mit Fehlerausgabe ...
echo.

"%PYTHON_EXE%" "%APP_DIR%main.py"

if %errorlevel% neq 0 (
    echo.
    echo Anwendung beendet mit Fehler.
)

pause