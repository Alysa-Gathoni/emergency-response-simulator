#pragma once
#include <array>
#include <vector>
#include "../core/Point.h"

// Ear clipping: splits any simple polygon (convex or concave, either
// winding) into n - 2 triangles.
//
//   An "ear" is three consecutive vertices a, b, c where
//     1. b is convex: cross(b - a, c - b) has the sign of the polygon's
//        winding (the shoelace area), and
//     2. no other remaining vertex lies inside triangle a b c.
//   Cutting the ear off leaves a simple polygon with one vertex fewer, and
//   every simple polygon with more than three vertices has at least two
//   ears (Meisters' two-ears theorem), so the loop always finishes.
//
// Cost: O(n^2) per polygon; our polygons have at most a few dozen
// vertices. The triangles feed the graphics API (GL_TRIANGLES), which can
// only fill convex primitives, and the SAT collision test, which needs
// convex pieces.
namespace Triangulation {

using Triangle = std::array<int, 3>;   // indices into the input polygon

// Shoelace formula: 1/2 * sum(x_i * y_(i+1) - x_(i+1) * y_i).
// Positive = clockwise on screen (y points down), negative = counter-clockwise.
float signedArea(const std::vector<Point>& polygon);

// Twice the signed area of triangle a b c (z of the 2D cross product).
float cross(Point a, Point b, Point c);

// Point inside or on the edges of triangle a b c (either winding).
bool insideTriangle(Point p, Point a, Point b, Point c);

// Index triples, n - 2 of them for a simple polygon with n vertices
// (fewer if repeated or collinear vertices are skipped).
std::vector<Triangle> earClip(const std::vector<Point>& polygon);

// The same triangles as a flat vertex list (3 points per triangle).
std::vector<Point> triangles(const std::vector<Point>& polygon);

}
