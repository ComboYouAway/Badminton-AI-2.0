# FieldTrackerApp - Create desktop shortcut
# This script creates a desktop shortcut for the FieldTrackerApp.

$appDir = Split-Path -Parent $MyInvocation.MyCommand.Define
$target = Join-Path $appDir "START.bat"
$workingDir = $appDir
$shortcutPath = [Environment]::GetFolderPath("Desktop") + "\FieldTrackerApp.lnk"

if (Test-Path $shortcutPath) {
    Write-Host "Shortcut already exists on desktop."
    exit 0
}

$ws = New-Object -ComObject WScript.Shell
$lnk = $ws.CreateShortcut($shortcutPath)
$lnk.TargetPath = $target
$lnk.WorkingDirectory = $workingDir
# Note: To set an icon, you would need an .ico file. We'll skip for now.
# $lnk.IconLocation = "path\to\icon.ico"
$lnk.Save()

Write-Host "Shortcut created on desktop."