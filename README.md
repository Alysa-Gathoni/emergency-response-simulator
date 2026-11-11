# Emergency Response Simulator

A 2D Nairobi-inspired emergency-response **training simulation** written in C++17 with legacy OpenGL.

![District overview](docs/screenshots/final-overview.png)

## Description

The trainee drives an ambulance through a fictional Nairobi district with live traffic, signalised junctions and pedestrians. Each call is a full shift task: reach the scene, treat the patient, drive them to hospital. Every call is scored on response time, collisions, red lights run and wrong turns.

The simulation is for **ambulance drivers, paramedics and dispatch trainees**, and their instructors. It trains route choice and safe handling of intersections under time pressure, not physical driving skill.

> Educational simulation only; not a real dispatch or navigation system.

## Group Members

| Student No. | Name |
|---|---|
| 170048 | Eyoel Abraham |
| 146510 | Wanjiru Nicole |
| 167016 | Macharia Alysa |
| 167997 | Muteti Scott |
| 159056 | Marsa Fatma |

## Features

- Five kinds of call (medical, road crash, fire, cardiac arrest, maternity) run as a state machine: en route → on scene → transport → completed or missed
- Live traffic (cars, matatus, motorcycles) on a 4 × 3 grid of signalised junctions; vehicles pull over for the siren
- Congestion-aware A* route guidance with wrong-turn recalculation
- Hierarchical ambulance model, driving physics, SAT collisions, skid marks and siren glow
- Day/night cycle, camera with culling, minimap
- Title menu, difficulty levels, scoring with grades and a saved high-score table
- Two renderers (legacy OpenGL and software) switchable at run time; resizable letterboxed window

## Graphics Techniques

All algorithms are our own C++; OpenGL and SFML only open the window and display the results.

| Technique | Where it is used |
|---|---|
| DDA and Bresenham lines | Road markings, outlines, debug vectors (**L** toggles) |
| Scan-line polygon fill | Roads, buildings, vehicles, HUD |
| Ear-clipping triangulation | Concave (L-shaped) buildings (**F** cycles fill modes) |
| Cohen–Sutherland, Sutherland–Hodgman, point clipping | Everything is clipped to the camera view (**C** shows the frame) |
| Midpoint circle and ellipse | Wheels, signals, pedestrians, markers, siren glow |
| Bézier and Catmull-Rom curves | Traffic routes and turns |
| 3×3 homogeneous transforms, scene graph | Vehicle placement; ambulance body → steering → wheels |
| Window-to-viewport transform | Minimap (**M**) |
| Dijkstra and A* | Route to the call and hospital (**V** compares them) |
| AABB + Separating Axis Theorem | Collisions with traffic and buildings |
| Colour interpolation | Day/night cycle (**T**) |
| OpenGL display list, matrix stack | Static city drawn once; vehicles via `glPushMatrix`-style transforms |

## How to Build and Run

Requirements: a C++17 compiler, CMake 3.16+, **SFML 2.5 or 2.6** (not 3.x) and OpenGL.

```bash
cmake -S . -B build
cmake --build build --config Release
./build/EmergencyResponseSimulator      # Windows: build\Release\EmergencyResponseSimulator.exe
ctest --test-dir build -C Release       # 21 unit tests
```

- **Windows:** put the SFML DLLs next to the `.exe`; if CMake cannot find SFML, add `-DSFML_DIR=<SFML>/lib/cmake/SFML`.
- **Linux:** `sudo apt install libsfml-dev libgl1-mesa-dev`
- **macOS:** `brew install sfml@2`

## Controls

| Key | Action |
|---|---|
| W A S D / arrows | Drive and steer |
| Space | Handbrake |
| E | Siren on / off |
| H | Controls panel |
| P / Esc | Pause / menu |
| R | Restart |
| T | Day / night |
| M | Minimap |
| O | OpenGL ↔ software renderer |
| L / F | Line and fill algorithm |
| G / B / C / V | Debug views: graphics, curves, clipping, pathfinding |
| F12 | Screenshot |

## Screenshots

| On scene | Night, siren glow | Congestion and A* (V) |
|---|---|---|
| ![On scene](docs/screenshots/cp3-on-scene.png) | ![Night](docs/screenshots/final-night-siren.png) | ![Congestion](docs/screenshots/final-congestion.png) |

| Transport with minimap | Ear clipping (F) | Title menu |
|---|---|---|
| ![Transport](docs/screenshots/final-transport.png) | ![Ear clipping](docs/screenshots/cp3-ear-clipping.png) | ![Title](docs/screenshots/final-title.png) |

A five-minute demo script is in [DEMO.md](DEMO.md).

## Project Structure

```
src/
├── core/          Point, colours, Mat3, camera, letterbox
├── lines/         DDA and Bresenham
├── polygons/      Polygons, transformations, ear clipping
├── rasterization/ Scan-line fill and clipping
├── shapes/        Midpoint circle and ellipse
├── curves/        Bézier and Catmull-Rom
├── city/          Roads, signals, buildings, pedestrians, day/night
├── simulation/    Ambulance, traffic, collisions, calls, pathfinding, scoring
├── render/        OpenGL and software renderers
├── ui/            HUD, fonts, menus, minimap
└── main.cpp
tests/             Unit tests (ctest)
```

## Known Limitations

- No sound; the siren is visual only.
- Route guidance accounts for congestion but not the current signal state.
- Built for SFML 2.x and legacy (compatibility-profile) OpenGL.
