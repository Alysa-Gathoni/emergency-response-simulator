# Emergency Response Simulator

A 2D Nairobi-inspired emergency-response training simulation written in C++.

## Description

The trainee drives an ambulance through a fictional Nairobi-inspired district
to reach emergency incidents. The simulation is aimed at ambulance drivers,
paramedics and emergency-dispatch trainees, and focuses on route choice,
intersection handling and balancing speed against safety.

## Group Members and Responsibilities

| Student No. | Name | Responsibility |
|---|---|---|
| 159056 | Marsa Fatma | 2D graphics & environment: line drawing, city layout |
| 146510 | Wanjiru Nicole | Vehicle systems: polygons, transformations, ambulance |
| 167016 | Macharia Alysa | Simulation algorithms: fill, clipping, collisions, pathfinding |
| 167997 | Muteti Scott | Traffic & intersections: circles/ellipses, traffic lights |
| 170048 | Eyoel Abraham | Curves & integration: Bézier/Catmull-Rom curves, civilian traffic, README |

## Graphics Techniques Implemented So Far

| Area | Algorithm(s) | Source | Used in the project for |
|---|---|---|---|
| Line drawing | DDA, Bresenham (toggle with **L**) | `src/lines/` | Road edges, lane markings, crosswalks, building outlines |
