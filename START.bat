@echo off
setlocal enabledelayedexpansion

:: FieldTrackerApp - Start script
:: Works from any location, uses %~dp0

set "APP_DIR=%~dp0"
set "VENV_DIR=%APP_DIR%.venv"
set "PYTHON_EXE=%VENV_DIR%\Scripts\python.exe"
set "PYTHONW_EXE=%VENV_DIR%\Scripts\pythonw.exe"

echo =========================================
echo  FieldTrackerApp
echo =========================================
echo.

:: --- 1. Check Python on PATH -----------------------------------------------
where python >nul 2>&1
if %errorlevel% neq 0 (
    echo FEHLER: Python ist nicht installiert oder nicht im PATH.
    echo.
    echo Bitte lade Python 3.10+ herunter:
    echo   https://www.python.org/downloads/
    echo.
    echo Installiere und setze "Add Python to PATH" beim Setup.
    echo.
    pause
    exit /b 1
)

:: --- 2. Create virtual environment on first run -----------------------------
if not exist "%VENV_DIR%" (
    echo Erstelle virtuelle Umgebung (.venv) ...
    python -m venv "%VENV_DIR%"
    if %errorlevel% neq 0 (
        echo FEHLER: Virtuelle Umgebung konnte nicht erstellt werden.
        pause
        exit /b 1
    )
)

:: --- 3. Upgrade pip and install requirements -------------------------------
if not exist "%PYTHON_EXE%" (
    echo FEHLER: Python in der virtuellen Umgebung nicht gefunden.
    pause
    exit /b 1
)

"%PYTHON_EXE%" -m pip install --upgrade pip >nul 2>&1

if not exist "%APP_DIR%requirements.txt" (
    echo FEHLER: requirements.txt fehlt im Ordner.
    pause
    exit /b 1
)

"%PYTHON_EXE%" -m pip install -r "%APP_DIR%requirements.txt"
if %errorlevel% neq 0 (
    echo FEHLER: Pakete konnten nicht installiert werden.
    pause
    exit /b 1
)

:: --- 4. Create desktop shortcut (once) -------------------------------------
"%PYTHON_EXE%" "%APP_DIR%create_shortcut.py" "%APP_DIR%main.py" "FieldTrackerApp"

:: --- 5. Start the application ----------------------------------------------
echo.
echo Starte FieldTrackerApp ...
"%PYTHONW_EXE%" "%APP_DIR%main.py"
if %errorlevel% neq 0 (
    echo.
    echo FEHLER: Die Anwendung ist nicht gestartet.
    echo Nutze START_DEBUG.bat fuer Fehlermeldungen.
    pause
)