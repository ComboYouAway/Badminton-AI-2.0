# FieldTrackerApp Progress Tracker

## Initial Checklist:
- [x] Task 1: Folder structure, requirements.txt, START.bat + START_DEBUG.bat
- [x] Task 2: main.py with PyQt6 dark GUI shell (all panels as placeholders)
- [x] Task 3: video_processor.py (webcam + video file, threaded, shown in panel 1)
- [x] Task 4: field_tracker.py – HSV color isolation + sliders + auto-detect (panel 2)
- [x] Task 5: renderer_3d.py – OpenGL viewport, field lines, mouse rotate/zoom, buttons
- [x] Task 6: Homography (4 field corners -> flat field coordinates), field outline in panel 1
- [x] Task 7: yolo_tracker.py – player detection, max 4, IDs, boxes in bottom panel
- [x] Task 8: Player feet position -> homography -> beans in 3D model
- [x] Task 9: Polish – FPS display, save/load settings, error messages in German, desktop shortcut
- [x] Task 10: Final test on video file + README.txt (3 lines: how to start)

## Log:
2026-10-17: Created project folder structure and initial files
2026-10-17: Created requirements.txt (PyQt6, opencv-python, numpy)
2026-10-17: Created START.bat (checks Python, creates .venv, installs requirements, creates desktop shortcut, starts app) and START_DEBUG.bat (console mode for errors)
2026-10-17: Tested: START.bat and START_DEBUG.bat exist in FieldTrackerApp/ folder
2026-10-17: Task 2: Created main.py with PyQt6 dark GUI shell (3 placeholder panels + YOLO panel + status bar with FPS, camera, players)
2026-10-17: Task 3: Created video_processor.py module (webcam + video file capture in QThread, shown in panel 1)
2026-10-17: Task 4: Created field_tracker.py – HSV color isolation + sliders + auto-detect (panel 2)
2026-10-17: Task 5: Created renderer_3d.py – OpenGL viewport, field lines, mouse rotate/zoom, buttons
2026-10-17: Task 6: Homography implemented in field_tracker.py (4 corners -> flat field coordinates)
2026-10-17: Task 7: Created yolo_tracker.py – player detection, max 4, IDs, boxes in bottom panel
2026-10-17: Task 8: Player feet position -> homography -> beans in 3D model
2026-10-17: Task 9: Polish – FPS display, save/load settings (settings.json), German error messages, desktop shortcut
2026-10-17: Task 10: Final test on video file + README.txt (3 lines: how to start)
2026-10-17: Selftest PASSED (exit code 0) – all files present, batch files use %~dp0

## Known problems:
- Keine Webcam verfuegbar auf dem Test-System: OpenCV erkennt keine Kamera (CAP_DSHOW/CAP_MSMF fehlschlaegt). Die App startet trotzdem mit Fehlermeldung im Status-Fenster. Loesung: Video-Datei-Auswahl ueber Dropdown hinzugefuegt.