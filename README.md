# Emergency Response Simulator

A 2D Nairobi-inspired emergency-response **training simulation** written in C++.

## Description

The trainee drives an ambulance through a fictional Nairobi-inspired urban district to reach emergency incidents. The district has live civilian traffic, traffic lights and buildings. The simulation is aimed at **ambulance drivers, paramedics and emergency-dispatch trainees**. It focuses on decision-making rather than physical driving skill:

- choosing efficient routes through a congested road network
- handling intersections and traffic signals under time pressure
- balancing response speed against safety

In later phases, each run will end with a response assessment covering response time, collisions and route efficiency, so trainees can review how they performed.

**Secondary audience:** emergency-services training instructors.

> This is an educational training simulation. It is not a real dispatch or navigation system, and it does not teach physical vehicle control.

## Group Members and Responsibilities

| Student No. | Name | Responsibility |
|---|---|---|
| 170048 | Eyoel Abraham | Curves & integration: Bézier/Catmull-Rom curves, civilian traffic, README |
| 146510 | Wanjiru Nicole | Vehicle systems: polygon representation, transformations, ambulance |
| 167016 | Macharia Alysa | Simulation algorithms: scan-line fill, clipping; next: collisions, pathfinding |
| 167997 | Muteti Scott | Traffic & intersections: midpoint circle/ellipse, traffic-light state machine |
| 159056 | Marsa Fatma | 2D graphics & environment: line drawing (DDA, Bresenham), city layout |

Testing and final delivery are shared responsibilities.

## Technologies and Tools

- **C++17**: all graphics algorithms are implemented by hand
- **SFML 2.5 / 2.6** (not 3.x): used only to open a window, read keyboard input and receive our computed pixels in one batch (`src/core/Raster.h`). SFML does not compute any lines, fills, clipping, circles or curves, and no SFML shape classes are used.
- **CMake 3.16+**: build system
- **Git / GitHub**: version control, issues and collaboration

## Graphics Techniques Implemented So Far

| Area | Algorithm(s) | Source | Used in the project for |
|---|---|---|---|
| Line drawing | DDA, Bresenham (toggle with **L**) | `src/lines/` | Road edges, lane markings, crosswalks, all outlines |
| Polygon representation & operations | Vertex-list polygons; translation, rotation and scaling about a pivot | `src/polygons/` | Buildings, hospital, ambulance and civilian vehicles; ambulance movement and turning |
| Polygon filling | Scan-line fill (half-open edge rule) | `src/rasterization/PolygonFill.cpp` | Road surfaces, buildings, hospital and vehicles |
| Line clipping | Cohen–Sutherland (region outcodes) | `src/rasterization/Clipping.cpp` | Viewport clipping demo (toggle with **C**) |
| Polygon clipping | Sutherland–Hodgman against a rectangular viewport | `src/rasterization/Clipping.cpp` | Viewport clipping demo (toggle with **C**) |
| Circles & ellipses | Midpoint circle, midpoint ellipse; solid versions filled row by row from the midpoint outline | `src/shapes/` | Solid wheels, traffic-light bulbs, tree canopies, pulsing incident marker |
| Curves | Cubic Bézier; Catmull-Rom spline (converted to Bézier segments); arc-length parameterisation | `src/curves/` | Civilian traffic routes at constant speed; vehicles face along the curve tangent (`atan2(dy, dx)`) |

Debug views show the algorithms working live:

- **G**: polygon vertices, scan-line spans, the ambulance centre and its heading vector
- **C**: original polygon and line segments (grey) and their clipped results
- **B**: spline paths, Bézier control polygons and control points, and Catmull-Rom knots (magenta)

## Project Structure

```
src/
├── core/          Point, colours, batched pixel output (Raster.h)
├── lines/         DDA and Bresenham, LineRenderer
├── polygons/      Polygon class and 2D transformations
├── rasterization/ Scan-line fill, Cohen–Sutherland and Sutherland–Hodgman clipping
├── shapes/        Midpoint circle and ellipse
├── curves/        Cubic Bézier, Catmull-Rom, arc-length spline paths
├── city/          City layout, traffic light, environment (trees)
├── simulation/    Ambulance, civilian traffic, incident marker, clipping demo
└── main.cpp       Main loop
```

## How to Compile and Run

Requirements: a C++17 compiler, CMake 3.16+ and SFML 2.5 or 2.6.

```bash
mkdir build
cd build
cmake ..
cmake --build .
./EmergencyResponseSimulator
```

On Windows, the executable is built in `build/Debug/` or `build/`, depending on the generator. The SFML DLLs must be next to the `.exe` or on the `PATH`.

### Controls

| Key | Action |
|---|---|
| W / ↑ | Drive forward |
| S / ↓ | Reverse |
| A / ← | Turn left |
| D / → | Turn right |
| L | Switch DDA / Bresenham line drawing |
| G | Toggle graphics debug view |
| C | Toggle clipping demonstration |
| B | Toggle Bézier / spline debug view |
| Esc | Exit |

## Current Progress

Phase 1 (graphics foundation) is complete. The city renders entirely with our own algorithms. The trainee can drive and turn the ambulance. A traffic light cycles through its states, an incident marker pulses at the destination, and three civilian vehicles follow Bézier spline routes.

### Next phases

- Collision detection between the ambulance, buildings and vehicles
- Incident generation with triage priority and target response times
- Destination detection, response timer and assessment/debrief screen
- Traffic-light interaction and civilian vehicles yielding to the siren
- Pathfinding (Dijkstra / A*) to show the optimal route in the debrief
- HUD showing incident type, distance, time and assessment
- A larger district with more intersections and matatus/motorcycles

## Known Limitations and Issues

- No collision detection yet: the ambulance can drive through buildings and other vehicles.
- Civilian traffic ignores the traffic light and the ambulance.
- The incident location is fixed, and there is no timer, assessment or HUD yet.
- Clipping is shown as a demonstration (key **C**) but is not yet used for a scrolling camera.
- The map is a single fixed screen (900×700) with one main intersection.
- Built against SFML 2.x; SFML 3 changed its API and is not supported yet.
