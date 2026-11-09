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
- **CMake 3.16+ / CTest**: build system and unit tests
- **Git / GitHub**: version control, issues and collaboration

## Graphics Techniques Implemented So Far

| Area | Algorithm(s) | Source | Used in the project for |
|---|---|---|---|
| Line drawing | DDA, Bresenham (toggle with **L**) | `src/lines/` | Road edges, lane markings, crosswalks, all outlines |
| Polygon representation & operations | Vertex-list polygons; translation, rotation and scaling about a pivot | `src/polygons/` | Buildings, hospital, ambulance and civilian vehicles; ambulance movement and turning |
| Polygon filling | Scan-line fill (half-open edge rule); spans submitted to SFML in one batch | `src/rasterization/PolygonFill.cpp` | Road surfaces, buildings, hospital, vehicles, signal housings |
| Line clipping | Cohen–Sutherland (region outcodes) | `src/rasterization/Clipping.cpp` | Viewport clipping demo (toggle with **C**) |
| Polygon clipping | Sutherland–Hodgman against a rectangular viewport | `src/rasterization/Clipping.cpp` | Viewport clipping demo (toggle with **C**) |
| Circles & ellipses | Midpoint circle, midpoint ellipse; solid versions filled row by row from the midpoint outline | `src/shapes/` | Solid wheels, 16 signal heads with three bulbs each, tree canopies, pulsing incident marker, camera flash |
| Curves | Cubic Bézier; Catmull-Rom spline (converted to Bézier segments); arc-length parameterisation | `src/curves/` | Traffic routes at constant speed; quarter-circle Bézier corners at every turn; vehicles face along the tangent (`atan2(dy, dx)`) |
| Signal control | Finite state machine per intersection with timing offsets | `src/city/SignalController.*` | Four coordinated intersections |
| Traffic behaviour | Stopping distance v² = 2·b·d, yellow dilemma zone, safe-gap following | `src/simulation/TrafficRules.*` | Civilian vehicles stop at red, queue and keep their distance |
| Segment intersection | Line-segment crossing test | `src/city/RedLightCamera.*` | Counting red lights run by the ambulance |

### Traffic and intersections

- A 2×2 grid of four signalised intersections on a 1100×720 map; roads are 140 px wide with two lanes. Every module derives its geometry from `RoadNetwork`, so the map is defined once.
- **Left-hand traffic** as in Kenya: the lane centre is the road centre plus `leftOf(direction) * 35`, where `leftOf(dx, dy) = (dy, -dx)` in screen coordinates.
- Each intersection runs its own signal state machine: `NS green (8 s) → NS yellow (2.5 s) → all red (1.5 s) → EW green → EW yellow → all red`. Neighbouring intersections are offset by 0, 8, 8 and 16 s, so the lights do not all change together.
- Civilian vehicles use the stopping relation **v² = 2·b·d**. A red light means stop. A yellow light means stop only if `v²/2b` fits before the stop line; otherwise the vehicle is in the dilemma zone and carries on. Vehicles follow a leader at `v = √(vL² + 2·b·gap)` and check a corridor ahead for the ambulance.
- Turns are cubic Bézier corners with control points 0.5523·r towards the corner (quarter-circle approximation). Left turns are tight and right turns wide, purely from the lane offsets.
- Traffic mix: 60% cars, 25% matatus (stop for 2.5 s at kerbside stages), 15% motorcycles; spawned from a fixed seed so runs are repeatable.
- The ambulance is never forced to stop, but a red-light camera counts every stop line it crosses on red.

Debug views show the algorithms working live:

- **G**: polygon vertices and scan-line spans, the ambulance centre and heading, stop lines in their signal colour, and each car's target-speed vector coloured by what is limiting it (free road, corner, signal, leader or stage)
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
├── city/          Road network, signal controllers, traffic lights, red-light camera, city layout, trees
├── simulation/    Ambulance, civilian traffic and its rules, vehicle types, incident marker, clipping demo
└── main.cpp       Main loop
tests/             Unit tests for the signals and traffic rules (run with ctest)
```

## How to Compile and Run

Requirements: a C++17 compiler, CMake 3.16+ and SFML 2.5 or 2.6.

```bash
mkdir build
cd build
cmake ..
cmake --build .
./EmergencyResponseSimulator
ctest -C Debug          # unit tests
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

Phase 1 (graphics foundation) is complete. The city renders entirely with our own algorithms.

**Checkpoint 2: traffic and intersections are complete.**

- Four signalised intersections with stop lines, zebra crossings and 16 signal heads
- Civilian traffic that stops at red, decides on yellow, queues with a safe gap and turns on Bézier corners in the correct lane
- Cars, matatus (with stages) and motorcycles
- Red-light camera counting violations by the ambulance
- Wide 140 px roads so the ambulance can be driven comfortably
- Unit tests for the signal state machines, lane geometry and traffic rules

### Next phases

- Transformation matrices and a hierarchical ambulance model with a proper driving model
- Collision detection between the ambulance, buildings and vehicles
- Incidents with time limits, arrival detection and route guidance
- Pedestrians at the crossings and a day/night cycle
- HUD showing incident type, distance, time and counters

## Known Limitations and Issues

- No collision detection yet: the ambulance can drive through buildings and other vehicles.
- Vehicles turning inside a junction do not yet yield to each other, so a brief overlap can occur rarely.
- The incident location is fixed, and there is no timer, assessment or HUD yet.
- Clipping is shown as a demonstration (key **C**) but is not yet used for a scrolling camera.
- The map is a single fixed screen (1100×720) with four intersections; there is no camera yet.
- Built against SFML 2.x; SFML 3 changed its API and is not supported yet.
