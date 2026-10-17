# Badminton-AI-2.0 — Milestone 1: Project Skeleton

## Dateiübersicht

```
Badminton-AI-2.0/
├── CMakeLists.txt                 # Build config (CMake + FetchContent für alle Deps)
├── README.md
├── CLAUDE.md                      # Projekt-Instructions
├── include/
│   ├── VideoProcessor.hpp         # Kamera + HSV-Isolation
│   ├── FieldTracker.hpp           # Homographie + Feld-Erkennung
│   ├── Renderer3D.hpp             # OpenGL + ImGui Fenster/Viewport
│   └── YoloTracker.hpp            # Stub für spätere YOLO-Integration
└── src/
    ├── main.cpp                   # Einstiegspunkt + Game-Loop
    ├── VideoProcessor.cpp
    ├── FieldTracker.cpp
    ├── Renderer3D.cpp
    └── YoloTracker.cpp            # Stub (keine echte Inferenz)
```

## Bausteine

1. **CMakeLists.txt**
   - Sucht OpenCV, OpenGL systemweit.
   - Holt GLFW, GLM, glad und Dear ImGui via `FetchContent`.
   - Linkeiert alles statisch, kopiert OpenCV-DLLs nach Build (Windows).

2. **VideoProcessor**
   - Öffnet Webcam (`CAP_DSHOW`).
   - `processColorIsolation()` liefert Binärmaske + durchsichtiges Ergebnis.

3. **FieldTracker**
   - Findet Konturen im Maskenbild.
   - Approximiert Ecken, sortiert sie als TL/TR/BR/BL.
   - Berechnet Homographie zur Standard-Badminton-Court-Größe (6.10 × 13.40 m).

4. **Renderer3D**
   - GLFW-Fenster (1280×720).
   - OpenGL-Kontext via glad.
   - Shader: einfarbige Linien/Cylinder.
   - ImGui-Dark-Mode-Layout:
     - Links: Video-Feed-Platz + HSV-Schieberegler.
     - Rechts: 3D-Viewport mit Orbit/Zoom.
     - Statuszeile: FPS / Kamera-Status / YOLO-Status.
   - Maus-Controls:
     - Links+Drag → Orbit um Y-Achse.
     - Scroll-Rad → Zoom.
     - Schalterbare Top-Down-Ansicht.

5. **YoloTracker**
   - Stub-Klasse (`loadModel()`/`detect()` immer erfolgreich/leer).
   - Bereit für spätere ONNX Runtime / LibTorch-Integration.

6. **main.cpp**
   - Startet alle Module.
   - Zeichnet ein statisches Badminton-Court-Raster in 3D.
   - Game-Loop mit ImGui + Rendering synchron.
