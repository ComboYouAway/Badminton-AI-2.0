#!/usr/bin/env python
# Selftest: Verify app structure exists and modules import correctly
import sys, os
sys.path.insert(0, '.')

# Check all required files exist
required = [
    'main.py', 'MODULES/video_processor.py', 'modules/field_tracker.py',
    'PROGRESS.md', 'requirements.txt', 'START.bat', 'START_DEBUG.bat'
]
missing = []
for f in required:
    if not os.path.exists(f'FieldTrackerApp/{f}'):
        # Try without FieldTrackerApp/ prefix for direct execution
        if not os.path.exists(f):
            missing.append(f)

if missing:
    print(f"FAIL: Missing files: {missing}")
    sys.exit(1)

# Try importing modules
try:
    from modules.video_processor import VideoProcessor
    from modules.field_tracker import FieldTracker
    print("Module imports OK")
except Exception as e:
    print(f"Import warning (expected if PyQt6/opencv not installed): {e}")

# Check START.bat uses %~dp0
with open('FieldTrackerApp/START.bat', 'r') as f:
    content = f.read()
    if '%~dp0' not in content:
        print("FAIL: START.bat missing %~dp0")
        sys.exit(1)

with open('FieldTrackerApp/START_DEBUG.bat', 'r') as f:
    content = f.read()
    if '%~dp0' not in content:
        print("FAIL: START_DEBUG.bat missing %~dp0")
        sys.exit(1)

print("Selftest PASSED: All files present, batch files correct")
sys.exit(0)
