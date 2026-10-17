# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project
FieldTrackerApp — Windows desktop app (Python 3.10+, PyQt6) for badminton field tracking. Pipeline: camera/video → HSV field isolation → homography → YOLO player tracking (max 4) → 3D OpenGL mini-model with colored player "beans". User-facing language is German; UI is dark-themed.

Repo: https://github.com/ComboYouAway/Badminton-AI-2.0 (default branch `main`). Only commit/push on explicit user request; user owns the repo.

## Commands
- Build/start: `python main.py` (from repo root; `modules/` must be importable). Use `START.bat` / `START_DEBUG.bat` at root for `.venv` setup + `pythonw` launch.
- Selftest (no camera needed): `python main.py --selftest`
- Requirements: `requirements.txt` (PyQt6, opencv-python, numpy, ultralytics). `START.bat` creates `.venv` on first run.
- Progress tracking: `PROGRESS.md` must be updated before/after each task (`[~]` → `[x]` + log entry + known problems).

## Architecture (big picture — needs multi-file reading)
- `main.py`: `MainWindow` (QMainWindow). Owns 3 top panels (live camera, HSV isolation, 3D model placeholder), YOLO output label, status bar with FPS/camera/player count. Creates `VideoProcessor` thread; connects `frame_ready` → `update_frame` and `error_occurred` → `handle_video_error` (German messages).
- `modules/video_processor.py`: `VideoProcessor` (QThread). Captures webcam or video file; emits frames via `pyqtSignal(np.ndarray)`.
- `modules/field_tracker.py`: `FieldTracker` + `FieldCorners`. HSV mask, contour, 4-corner homography (`get_perspective_transform` / `warpPerspective`) to flat field coords. Auto-detect button wired but partially unimplemented.
- `modules/yolo_tracker.py`: YOLOv8n (`ultralytics`), max 4 persons, ID tracking, boxes for bottom YOLO panel.
- `modules/renderer_3d.py`: `QOpenGLWidget`, field lines, player beans (capsules/spheres), mouse rotate/zoom, zoom/top-view/reset buttons.
- Invariant: video capture and YOLO run in worker threads; GUI updates only via Qt signals (`frame_ready`, `error_occurred`). Never block the main thread.

## Rules / Constraints
- Stack is fixed (PyQt6, opencv-python, numpy, ultralytics, QOpenGLWidget + PyOpenGL). No extra libraries.
- German error messages; friendly failures (missing camera, no GPU). CPU fallback if GPU unavailable.
- One task at a time; app must start after every change; explain in 2–3 German sentences + stop for "weiter" (per MASTER_PROJECT_GUIDE).
- `MASTER_PROJECT_GUIDE.txt` is the source of truth for UI layout, task list, and working rules.
- `README.txt`: 3-line German start instructions (double-click `START.bat`).
- `_backup_alt/CLAUDE.md`: older backup; current project rules supersede it.
