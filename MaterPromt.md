# MASTER PROMPT: FieldTrackerApp (Badminton court tracker)

You are building a Windows desktop app in Python. The user is NOT a programmer. Be careful, work in small steps, verify every step, and never leave the app in a broken state. Talk to the user in simple German. Code and comments in English. All UI texts in German.

Read this whole file once before you start. Then do exactly what the section "START NOW" at the bottom says.

---

## 0. PROJECT CONTEXT (facts, do not guess around them)

- **Sport:** Badminton, indoor hall, doubles (up to 4 players).
- **Floor:** brown wooden floor with GREEN court lines. The color isolation must isolate the GREEN LINES (not the floor).
- **Camera:** Sony compact camera on a tripod, about 6 m high, 1080p at 50 fps, HDMI to a capture card (Windows sees it as a webcam). The camera has NO clean HDMI: on-screen icons/overlays are visible in the image -> a crop mode and ignore zones are required.
- **Camera does not move during a game**, but it stands slightly differently every time it is set up -> the court detection must be adaptive (automatic, with a manual fallback).
- **Far parts of the court are hard to see**, near lines are clear. The full court must be reconstructed from the visible lines using the known court dimensions (homography).
- **At home** the user simulates the hall: a tablet plays a recorded video via HDMI into the capture card. Also support plain video files.
- **Hardware the app must run on:** Laptop with NVIDIA GeForce RTX (probably 5050, brand new), PC with GTX 1080 Ti, friend's PC with AMD Radeon. Must work on all three, with automatic CPU fallback.
- **Shuttlecock tracking: NOT now.** Only players. (Maybe much later, see optional Phase J.)

## 1. TECH STACK (fixed, do not swap)

- Python 3.10+ (target 3.11), Windows 10/11
- PyQt6 (dark modern stylesheet)
- opencv-python (NOT opencv-contrib), numpy, scipy
- 3D: PyQt6 `QOpenGLWidget` + PyOpenGL, OpenGL 2.1 compatibility (fixed-function) on purpose: it works on NVIDIA, AMD and Intel
- Detection runtime: **onnxruntime-directml** (works on NVIDIA, AMD, Intel under Windows; includes CPU provider as fallback). Do NOT install plain `onnxruntime` or `onnxruntime-gpu` together with it.
- YOLOv8n (Ultralytics) is used ONLY to export the model to ONNX once (`tools/export_model.py`). The running app must NOT import `torch` or `ultralytics` (that is what makes startup slow).

## 2. LAYOUT OF THE WINDOW

Top row, 3 panels side by side:
1. **Live-Kamera** (with crop mode, ignore zones, court overlay)
2. **Farbisolation** (white = line pixels, black = rest) + HSV sliders + "Linienfarbe per Klick"
3. **3D-Modell** (OpenGL): court lines only, players as colored "beans" (capsules). Mouse: left drag = rotate, right/middle drag = pan, wheel = zoom. Buttons: Zoom +, Zoom -, Von oben, Seite, Isometrisch, Reset.

Bottom row: **Spieler-Tracking** (camera image with YOLO boxes + IDs P1-P4) and a small control column (device GPU/CPU, calibration status, buttons).
Status bar: FPS capture, FPS processing, device used (GPU/CPU), calibration confidence, number of players.

There is NO separate 2D view. "Von oben" is just a camera preset of the 3D view.

## 3. FOLDER STRUCTURE

```
FieldTrackerApp/
├── START.bat
├── START_DEBUG.bat
├── create_shortcut.ps1
├── build_exe.bat
├── PROGRESS.md
├── README.txt
├── requirements.txt
├── requirements-export.txt
├── settings.json            (created at runtime)
├── calibration.json         (created at runtime)
├── main.py
├── styles/dark.qss
├── models/                  (yolov8n.onnx lands here)
├── logs/
├── test_images/             (user puts real screenshots here)
├── tools/
│   ├── export_model.py
│   └── synth.py             (synthetic badminton test frames)
├── tests/
└── modules/
    ├── __init__.py
    ├── constants.py         (court dimensions, colors)
    ├── settings.py
    ├── logger.py
    ├── video_processor.py
    ├── field_tracker.py     (color isolation + homography)
    ├── yolo_tracker.py      (ONNX detection + player tracker)
    ├── renderer_3d.py
    ├── pipeline.py          (connects everything)
    └── widgets.py           (panels, dialogs, crop overlay)
```

## 4. RULES (MANDATORY, in this order of importance)

**R1. PROGRESS.md.** Maintain `PROGRESS.md` in the project root at ALL times.
- On the first run, generate it from the task list in section 6 (every task as `- [ ] Task N: ...`, grouped by phase).
- Before starting a task: set it to `[~]`. After finishing AND testing it: set it to `[x]`. Update the file BEFORE and AFTER every task.
- Keep two extra sections at the bottom: `## Log` (one line per finished task: date, what, how to test) and `## Known problems` (anything unresolved).

**R2. The app must start after every task.** Placeholders are fine, a crash is not.

**R3. Small steps.** One task at a time. Keep files under ~400 lines; split if bigger. No clever abstractions.

**R4. Never block the GUI thread.** Camera, color isolation, YOLO and model loading run in worker threads/QThreads. Communicate with Qt signals. Always process only the LATEST frame (drop old ones).

**R5. Startup speed.** The window must be visible in under 5 seconds. Use lazy imports, a splash screen, and load the ONNX model in the background. Log the time to window and time to model ready.

**R6. Friendly errors.** Never crash on: camera not found, wrong format, GPU not available, model missing. Show a short German message and log details to `logs/app.log`.

**R7. Self test.** `python main.py --selftest` must exist from Phase B on and must grow with the app (see Task 26, 100 and 143). It runs headless (`QT_QPA_PLATFORM=offscreen`) and exits with code 0 only if everything passes. Run it at the end of every phase.

**R8. Git.** `git init` in Task 2. Commit after every phase with message `Phase X done`.

**R9. Phase end.** At the end of each phase: run the selftest, update PROGRESS.md, commit, then print `PHASE X FERTIG` plus 3 sentences in German on what the user can click to test. Then continue automatically ONLY if a `/goal` is active; otherwise stop and wait for "weiter".

**R10. If stuck.** If the same error persists after 3 different attempts: write it under `Known problems` in PROGRESS.md and continue with the next task. Do not loop forever.

**R11. No guessing about the user's hardware or the real hall.** If something needs a real screenshot, camera test or a decision, ask ONE short question in German.

**R12. Do not touch** the folder `_backup_alt` (old project). Do not read from it unless the user asks.

**R13. Do not add libraries** beyond the stack in section 1 without asking.

## 5. COURT MODEL (use exactly these numbers, all in meters)

Coordinate system: x along the length (0 to 13.40), y across the width (0 to 6.10). Net at x = 6.70.

- Outer boundary (doubles): rectangle (0,0) - (13.40, 6.10)
- Singles side lines: y = 0.46 and y = 5.64 (from x=0 to x=13.40)
- Short service lines: x = 6.70 - 1.98 = 4.72 and x = 6.70 + 1.98 = 8.68 (full width y = 0 to 6.10)
- Doubles long service lines: x = 0.76 and x = 12.64 (full width)
- Center line: y = 3.05, from x = 0 to 4.72 and from x = 8.68 to 13.40
- Net: x = 6.70, height 1.55 m at the posts, posts at y = -0.0 and y = 6.10 (draw slightly outside the lines)
- Line width: 0.04 m

Put all of this in `modules/constants.py` as data (list of line segments + list of named key points) so renderer, detection and tests use the same source.

The court is symmetric (180 degree rotation, mirroring). The user must be able to choose once which side is "Seite A" so the orientation in 3D stays stable.

## 6. TASK LIST (generate PROGRESS.md from exactly this)

### PHASE A: Restart and foundation
- [ ] Task 1: Check that old project files are in `_backup_alt` (if not, ask the user); then create the folder structure from section 3
- [ ] Task 2: `git init`, `.gitignore` (venv, logs, settings.json, models/*.onnx, __pycache__)
- [ ] Task 3: `requirements.txt` (PyQt6, PyOpenGL, opencv-python, numpy, scipy, onnxruntime-directml)
- [ ] Task 4: `requirements-export.txt` (ultralytics, onnx; ONLY for export)
- [ ] Task 5: Generate `PROGRESS.md` from this task list
- [ ] Task 6: `START.bat`: check Python (friendly message + download link if missing), create `.venv` on first run, install requirements, start `main.py` with `pythonw` (no console), work from any folder (`%~dp0`)
- [ ] Task 7: `START_DEBUG.bat`: same but with console and `pause` at the end
- [ ] Task 8: `create_shortcut.ps1`: desktop shortcut with icon pointing to START.bat
- [ ] Task 9: `modules/logger.py`: rotating log in `logs/app.log`
- [ ] Task 10: `modules/settings.py`: load/save `settings.json` with defaults, safe against missing/broken file
- [ ] Task 11: `modules/constants.py`: court model from section 5 as data
- [ ] Task 12: `README.txt` (3 simple German lines: how to start, how to calibrate, where logs are)

### PHASE B: GUI shell
- [ ] Task 13: `main.py`: splash screen, lazy imports, startup time logging
- [ ] Task 14: `styles/dark.qss`: modern dark theme (rounded corners, one accent color, readable fonts, styled sliders/buttons/comboboxes)
- [ ] Task 15: main window with splitters: 3 panels top, tracking panel + control column bottom
- [ ] Task 16: reusable `Panel` widget (title bar, small FPS label, double-click = fullscreen toggle)
- [ ] Task 17: status bar fields (capture FPS, processing FPS, device, calibration confidence, player count)
- [ ] Task 18: menu/toolbar: Quelle, Kalibrieren, Einstellungen, Hilfe
- [ ] Task 19: settings dialog with tabs (Kamera, Feld, YOLO, 3D, Allgemein), saving to settings.json
- [ ] Task 20: remember window size, position and splitter sizes
- [ ] Task 21: global exception hook: log + German error dialog, app stays open
- [ ] Task 22: app icon generated in code (simple shape), no external file needed
- [ ] Task 23: placeholder content in every panel
- [ ] Task 24: keyboard shortcuts: F11 fullscreen, Space pause, C crop mode, K calibrate, R reset 3D, T top view
- [ ] Task 25: "Über"-dialog with versions (Python, Qt, OpenCV, onnxruntime, OpenGL renderer string)
- [ ] Task 26: `--selftest` skeleton (creates window offscreen, closes, exit code 0)

### PHASE C: Video input (also fixes: slow open + black-and-white picture)
- [ ] Task 27: scan camera indices 0-9 in a background thread, list found devices with resolution in a dropdown
- [ ] Task 28: capture thread (`QThread`), signal `frame_ready`, keep only the latest frame, buffer size 1
- [ ] Task 29: open capture card with `cv2.CAP_DSHOW`, request `MJPG`, 1920x1080, 50 fps
- [ ] Task 30: fallbacks: MSMF backend with env `OPENCV_VIDEOIO_MSMF_ENABLE_HW_TRANSFORMS=0`, then default backend; log which one worked
- [ ] Task 31: open the camera asynchronously with timeout (no frozen window while opening)
- [ ] Task 32: FIX black-and-white: log `frame.shape`, `dtype`; make sure frames have 3 channels; set `CAP_PROP_CONVERT_RGB` to 1; if gray, convert; verify panel 1 shows the ORIGINAL frame and not the mask
- [ ] Task 33: correct BGR to RGB conversion into `QImage` (`Format_RGB888`, correct `bytesPerLine`, no memory leak)
- [ ] Task 34: video file source (mp4/avi/mkv), looping, for tests
- [ ] Task 35: source dropdown (capture card / video file) switchable while running
- [ ] Task 36: auto-reconnect if the capture drops, with status text
- [ ] Task 37: downscale frames for display in the worker thread (`INTER_AREA`), not in the GUI thread
- [ ] Task 38: smoothed FPS counter utility used by all stages
- [ ] Task 39: pause / freeze frame
- [ ] Task 40: snapshot button: saves current frame into `test_images/`
- [ ] Task 41: **Crop-Modus**: draw/resize a rectangle on the live view with the mouse (handles at corners and edges)
- [ ] Task 42: apply crop BEFORE all processing; keep a mapping crop <-> original coordinates
- [ ] Task 43: **Ignore zones**: draw several rectangles (e.g. camera icons); those pixels are blanked before color isolation and detection
- [ ] Task 44: numeric crop fields + "Zurücksetzen" button
- [ ] Task 45: crop and ignore zones saved in settings.json
- [ ] Task 46: measure and log time from click to first frame; target under 3 s

### PHASE D: Color isolation (green lines on brown wood)
- [ ] Task 47: work on a downscaled copy (width about 960 px) for speed
- [ ] Task 48: HSV conversion + 6 sliders (H/S/V min/max) with live mask
- [ ] Task 49: default preset "Grüne Linien auf Holzboden"
- [ ] Task 50: **"Linienfarbe per Klick"**: user clicks on a line in the live view, app samples the neighborhood and sets HSV range with adjustable tolerance
- [ ] Task 51: "Auto" button: suggests HSV range from histogram of strongly saturated green pixels
- [ ] Task 52: morphological cleanup (open/close) with adjustable kernel size
- [ ] Task 53: panel 2 shows the mask; checkbox "Overlay": show mask tinted over the live image
- [ ] Task 54: lighting robustness: CLAHE on the V channel (optional checkbox)
- [ ] Task 55: apply crop and ignore zones to the mask
- [ ] Task 56: slow re-adaptation of the color range every N seconds (optional checkbox, off by default)
- [ ] Task 57: show a mask quality value (ratio of line pixels, number of connected components)
- [ ] Task 58: remove tiny blobs (area filter) and keep elongated structures
- [ ] Task 59: save HSV settings
- [ ] Task 60: test: mask on a synthetic frame finds the lines (selftest)
- [ ] Task 61: performance: mask step under 5 ms at 960 px width, logged
- [ ] Task 62: debug toggles (show contours, show component stats)

### PHASE E: Court detection and homography (hardest part, go slowly)
- [ ] Task 63: `tools/synth.py`: render a badminton court (exact model from section 5) on a brown wood texture, apply a KNOWN random homography (camera about 6 m high, side view), add noise, blur, partial cropping, overlay icons; returns image + ground-truth H
- [ ] Task 64: line segment detection on the mask with `cv2.HoughLinesP` (parameters adjustable)
- [ ] Task 65: merge collinear/nearby segments into longer lines
- [ ] Task 66: split lines into two families (along the court length / across the width) using angle and vanishing point estimate
- [ ] Task 67: estimate the two vanishing points with RANSAC
- [ ] Task 68: sort lines inside each family by position (so neighbors are known)
- [ ] Task 69: compute intersections between the families (candidate corner points)
- [ ] Task 70: generate hypotheses: assign detected lines to model lines (using ordering and known spacing ratios) and compute H with `cv2.findHomography`
- [ ] Task 71: **scoring**: project the model lines with H onto the image and measure how many model-line pixels hit mask pixels (chamfer / distance transform); higher is better
- [ ] Task 72: choose the best hypothesis; require a minimum score, otherwise "Feld nicht erkannt"
- [ ] Task 73: support partial visibility: accept hypotheses with at least 4 well spread, non-collinear correspondences
- [ ] Task 74: refine H by local optimization of the score (scipy) around the best hypothesis
- [ ] Task 75: temporal smoothing of H (smooth the 4 projected corner points, then recompute H)
- [ ] Task 76: **Lock-Button**: once good, freeze the calibration (camera is static); unlocked = keeps refining slowly
- [ ] Task 77: **manual calibration dialog**: schematic court on the left, camera image on the right, user clicks at least 4 known court points in both
- [ ] Task 78: manual points snap to the nearest detected line intersection if one is close
- [ ] Task 79: draggable corner/point handles with live reprojection of the court lines
- [ ] Task 80: draw projected court overlay on the live view (toggle)
- [ ] Task 81: confidence meter (0-100 percent) in UI and status bar
- [ ] Task 82: inverse transform: image point -> court coordinates in meters
- [ ] Task 83: court polygon in image space (with margin) for the player filter
- [ ] Task 84: Seite A/B + orientation choice (resolve 180 degree / mirror ambiguity once, store it)
- [ ] Task 85: camera-moved warning: if score stays low for more than 5 s while locked, show "Kamera bewegt? Neu kalibrieren"
- [ ] Task 86: save calibration (H, manual points, orientation, name e.g. "Halle Nord") to `calibration.json`
- [ ] Task 87: load last calibration on start, button "Letzte Kalibrierung verwenden"
- [ ] Task 88: selftest: auto detection on 20 synthetic frames with random H; mean corner reprojection error under 8 px at 1080p, success rate at least 90 percent
- [ ] Task 89: test script `tools/check_images.py`: runs detection on all images in `test_images/` and writes annotated results to `test_images/out/`
- [ ] Task 90: `README_KALIBRIERUNG.txt` in very simple German (3-6 steps)
- [ ] Task 91: performance: detection runs only when unlocked or on demand; under 40 ms when running
- [ ] Task 92: PHASE E review: write into PROGRESS.md what works on synthetic data and what still needs real hall screenshots

### PHASE F: YOLO player tracking (ONNX, GPU/CPU)
- [ ] Task 93: `tools/export_model.py`: download/export `yolov8n.pt` to `models/yolov8n.onnx` (imgsz 640, simplify, opset suitable for onnxruntime)
- [ ] Task 94: first-run helper: if the model is missing, run the export with a progress dialog (uses requirements-export.txt in the same venv); clear German message if it fails
- [ ] Task 95: ONNX Runtime session with providers `[DmlExecutionProvider, CPUExecutionProvider]`
- [ ] Task 96: preprocessing: letterbox to 640, BGR->RGB, NCHW float32 /255
- [ ] Task 97: postprocessing: decode YOLOv8 output (1x84x8400, transpose), class "person" only, confidence threshold, NMS with `cv2.dnn.NMSBoxes`
- [ ] Task 98: map boxes back to crop-frame coordinates (undo letterbox)
- [ ] Task 99: device selector in UI: Auto / GPU (DirectML) / CPU; always show which device is really used
- [ ] Task 100: selftest: model loads on CPU, one inference runs on a synthetic frame, output shapes valid
- [ ] Task 101: automatic fallback to CPU if DirectML fails to create or run a session (log + message)
- [ ] Task 102: "Benchmark" button: ms per frame for each available device
- [ ] Task 103: detection runs in its own worker thread, latest frame only
- [ ] Task 104: adaptive frame skipping to keep latency low (target about 25 processed fps even if capture is 50 fps)
- [ ] Task 105: load the model in the background; app is usable before the model is ready; status shows "Modell lädt..."
- [ ] Task 106: model choice dropdown yolov8n (default) / yolov8s (needs second export), because players are small at 6 m height
- [ ] Task 107: optional tiling or higher input size (imgsz 960) switch for small far players
- [ ] Task 108: filter: only persons whose FOOT POINT (bottom center of box) lies inside the court polygon plus margin (removes spectators, referees outside)
- [ ] Task 109: keep at most 4 players: highest confidence inside the court, with hysteresis to avoid flicker
- [ ] Task 110: foot point -> court coordinates in meters via inverse H
- [ ] Task 111: simple tracker: stable IDs P1-P4, matching by court-coordinate distance (+ IoU), keep a lost player for N frames, reuse the same ID when he returns
- [ ] Task 112: smoothing with a 2D constant velocity Kalman filter in court coordinates
- [ ] Task 113: fixed color per ID (P1 red, P2 blue, P3 yellow, P4 green-cyan; colors configurable)
- [ ] Task 114: draw boxes, IDs and court coordinates on the tracking panel
- [ ] Task 115: reject boxes with implausible size for their position (perspective-aware min/max height)
- [ ] Task 116: selftest: tracker keeps IDs stable on synthetic moving points, including a short gap
- [ ] Task 117: PHASE F review: log average inference ms on the device used

### PHASE G: 3D viewport
- [ ] Task 118: set `QSurfaceFormat` BEFORE creating `QApplication`: OpenGL 2.1 compatibility, depth buffer 24, 4x MSAA
- [ ] Task 119: `renderer_3d.py`: `QOpenGLWidget` with `initializeGL`, `resizeGL`, `paintGL`; all GL calls in try/except with logging
- [ ] Task 120: orbit camera (yaw, pitch, distance, target); left drag rotate, right/middle drag pan, wheel zoom, limits to avoid flipping
- [ ] Task 121: draw floor plane (dark, slightly lighter than background)
- [ ] Task 122: draw all court lines from `constants.py` as flat bright strips with anti-aliasing
- [ ] Task 123: draw net with posts at x = 6.70 (thin translucent mesh, height 1.55 m)
- [ ] Task 124: optional grid and axes (toggle)
- [ ] Task 125: bean = capsule (cylinder + 2 spheres via GLU quadrics), height about 1.7 m (adjustable), color by ID
- [ ] Task 126: soft shadow circle under each bean
- [ ] Task 127: labels P1-P4 above beans via `QPainter` overlay
- [ ] Task 128: smooth movement: interpolate bean positions every frame (60 fps timer), independent of detection FPS
- [ ] Task 129: buttons: Zoom +, Zoom -, Von oben, Seite, Isometrisch, Reset
- [ ] Task 130: animated transition between camera presets (about 0.4 s)
- [ ] Task 131: optional trails (last N seconds) per player, toggle
- [ ] Task 132: simple lighting (one light), colors readable in dark theme
- [ ] Task 133: orientation of the court in 3D follows "Seite A/B" from Task 84
- [ ] Task 134: demo mode: 4 beans walk around randomly (to test the renderer without camera)
- [ ] Task 135: screenshot button for the 3D view
- [ ] Task 136: only OpenGL 2.1 features, no extensions; log GL version/vendor/renderer string at start
- [ ] Task 137: performance: paintGL under 3 ms, logged
- [ ] Task 138: selftest: scene math (camera matrices, bean positions) is testable without a GL context; create the GL widget only if the platform allows it, otherwise skip with a warning (not a failure)

### PHASE H: Integration
- [ ] Task 139: `pipeline.py`: one controller object that owns the threads and connects all signals
- [ ] Task 140: start/stop pipeline buttons and states (idle, calibrating, tracking)
- [ ] Task 141: end-to-end flow: capture -> crop/ignore -> color isolation -> court detection -> YOLO -> court coordinates -> 3D beans
- [ ] Task 142: timestamps on frames; drop stale results; show end-to-end latency in the status bar
- [ ] Task 143: full selftest: synthetic video through the whole pipeline (without real GL if not possible), exit code 0
- [ ] Task 144: save ALL settings (crop, ignore zones, HSV, device, model, orientation) and restore on start
- [ ] Task 145: 60 second stability run: memory must not grow (log RSS at start/end)
- [ ] Task 146: startup check: window under 5 s, model ready log line, camera ready log line
- [ ] Task 147: `build_exe.bat`: optional PyInstaller one-folder build (so the app can start without Python)
- [ ] Task 148: final cleanup: remove dead code, update README.txt, make sure PROGRESS.md is fully `[x]` or documented in Known problems
- [ ] Task 149: final report to the user (German): what works, what needs real-hall testing, how to start

### PHASE J (optional, ONLY after Task 149, and ONLY after asking the user)
- [ ] Task 150: heatmap on the 3D floor (where players spent time)
- [ ] Task 151: CSV export of tracks (time, player, x, y)
- [ ] Task 152: stats: distance walked and speed per player
- [ ] Task 153: replay of recorded tracks
- [ ] Task 154: shuttlecock tracking (probably needs a better camera; ask first)

---

## 7. TECHNICAL NOTES (use them, they are decisions, not suggestions)

- **Black-and-white bug:** common causes are showing the mask instead of the frame, a capture card delivering a gray/YUV frame with `CONVERT_RGB` off, or wrong `QImage` format. Check all three in Task 32.
- **Slow opening of the capture card:** the default MSMF backend is slow. Use DSHOW + MJPG first (Task 29-31).
- **Slow app start:** caused by importing `torch`/`ultralytics` and loading the model in the GUI thread. Neither may happen in the running app (R5).
- **GPU strategy:** DirectML covers NVIDIA (incl. brand new RTX 50 series without needing a special PyTorch build), AMD and Intel. CPU is the guaranteed fallback. The device in use must always be visible in the UI.
- **Scoring is the key to court detection:** many hypotheses are cheap to generate; the projected-model-vs-mask score decides. Do not trust single Hough lines.
- **Static camera = calibrate once, then lock.** Re-calibrate only on user request or the camera-moved warning.
- **Players are small** (6 m height, 1080p). If detection is weak: larger input size (Task 107), yolov8s (Task 106), crop to the court region before YOLO.
- **Synthetic tests prove the logic, not the hall.** Real screenshots from the user go into `test_images/`; use `tools/check_images.py` on them as soon as they exist.

## 8. START NOW

1. Read the whole file. Do not ask questions you can answer from it.
2. Do Phase A (Tasks 1-12): generate `PROGRESS.md` first, then work through the tasks, marking `[~]` and `[x]`.
3. Apply R9: at the end of the phase run the selftest (once it exists), commit, print `PHASE A FERTIG` with what to test, then stop and wait for "weiter" (unless a `/goal` is active).