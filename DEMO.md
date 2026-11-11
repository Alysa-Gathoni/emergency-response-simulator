# Demo Script (5 minutes)

Run a Release build in a window of about 1280 × 800. Start on the title menu, difficulty **NORMAL**. Every presenter shows their own part and can explain the maths and the code behind it.

## 1. What and why (30 s)

- A training simulation for ambulance drivers and dispatch trainees: route choice, intersections and safety under time pressure in a fictional Nairobi district.
- Every graphics algorithm is our own C++ (lines, fill, clipping, circles, curves, transformations, triangulation); OpenGL and SFML only open the window and put our pixels, spans and matrices on screen.

## 2. Algorithm gallery (2 min, about 25 s each)

Start a shift (**Enter**), let the controls panel fold away, then:

| Presenter | Keys | Show | Explain |
|---|---|---|---|
| Interface & integration | **O**, **L** | Same frame through OpenGL and through the software renderer; Bresenham → DDA → `GL_LINES` | `IRenderer` with a view matrix and model stack; Bresenham's integer error term vs DDA's float steps; the fixed 60 Hz step; drag the window corner to show the letterbox `scale = min(W/1100, H/720)` |
| Vehicle systems | drive (**W A S D**, **Space**), **G** | Steering wheels, flashing light bar, skid marks under the handbrake, scene-graph axes | `world(child) = world(parent) · local(child)`; bicycle model `dθ/dt = v/L · tan δ`; impulse response along the contact normal; skid marks from rear-wheel world positions |
| 2D graphics & environment | **F**, **C**, **M**, **T** | Ear-clipped L-shaped buildings; the clip frame cutting lines, polygons and pixels; minimap; night with headlight cones and the siren glow | Ear test (convex corner, no vertex inside); display list for the static city; Cohen–Sutherland / Sutherland–Hodgman / point clipping; window-to-viewport `T·S·T`; keyframed colour interpolation |
| Traffic & intersections | **E**, **G**, **B** | Traffic pulling over for the siren (and not when **E** mutes it); signal phases and stop lines; Bézier turns | Signal state machine and green-wave offsets; `v² = 2·b·d` stopping; junction yield rule; congestion factor per road section |
| Simulation algorithms | **V**, drive off the route | Dijkstra rings vs A* discs with node counts; the route avoiding a jam; *WRONG TURN – RECALCULATING* | A* with the straight-line heuristic (admissible because weights ≥ length); SAT vs AABB broad phase (**G** shows both); the call state machine |

## 3. Live call (2 min)

1. **Esc**, **Esc** back to the menu; set **HARD**; start a shift.
2. Follow the route with the siren on; point at traffic pulling over and the minimap.
3. Stop on the marker: the hold ring, then the treatment ring (*CREW AT WORK*).
4. Drive the patient to the hospital (blue route), stop in the bay.
5. Result screen: response time against par, penalties, difficulty bonus, grade. **Enter** for the next call.

## 4. Debug views and robustness (30 s)

- **G** traffic debug: each car's target-speed vector coloured by what limits it (light blue = yielding to the siren).
- **P** pause, **R** restart, minimise and restore the window, resize it.
- `ctest`: 21 test executables, including a headless end-to-end call.

## Headless evidence (no presenter needed)

```bash
EmergencyResponseSimulator --screenshot call.png 12 !        # demo driver on a call
EmergencyResponseSimulator --screenshot night.png 4 TT!      # night, siren glow
EmergencyResponseSimulator --screenshot jam.png 30 EV        # congestion overlay
EmergencyResponseSimulator --screenshot wide.png 3 "" 1600x700
```
