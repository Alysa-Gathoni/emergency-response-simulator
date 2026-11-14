#pragma once
#include <vector>
#include "../core/Point.h"

// Collision geometry (pure C++, no rendering).
//
// Broad phase: axis-aligned bounding boxes (cheap, loose for rotated shapes).
// Narrow phase: the Separating Axis Theorem on convex polygons (exact).
//
// Axis-aligned bounding boxes:
//
// Two boxes overlap exactly when their intervals overlap on BOTH axes:
//     a.minX < b.maxX && b.minX < a.maxX && a.minY < b.maxY && b.minY < a.maxY
// When they do, the cheapest way to separate them is to move along the
// axis with the smaller penetration depth (minimum translation vector).
struct AABB {
    float minX;
    float minY;
    float maxX;
    float maxY;

    float width() const { return maxX - minX; }
    float height() const { return maxY - minY; }
    Point centre() const { return {(minX + maxX) * 0.5f, (minY + maxY) * 0.5f}; }
};

namespace Collision {

// Tightest AABB around a set of points (e.g. a rotated polygon).
AABB boundsOf(const std::vector<Point>& points);

bool overlaps(const AABB& a, const AABB& b);

// Vector that moves `a` out of `b` along the axis of least penetration.
// Zero vector if the boxes do not overlap.
Point minimumTranslation(const AABB& a, const AABB& b);

// Keeps a box inside the world rectangle; returns the correction applied.
Point keepInside(const AABB& box, const AABB& world);

// Separating Axis Theorem for two convex polygons (either winding).
//
// Two convex shapes are apart exactly when some line separates them, and
// it is enough to try the normals of their edges as the direction of that
// line. For each axis n both polygons are projected onto n (dot products
// of the vertices give an interval [min, max]); if the intervals do not
// overlap on some axis, there is no collision. Otherwise the axis with
// the smallest overlap is the contact normal and the overlap is the depth.
struct Contact {
    bool hit{false};
    Point normal{0.0f, 0.0f};   // unit, points from b towards a
    float depth{0.0f};          // move a by normal * depth to separate
};

// Projection interval of a polygon on a unit axis.
void project(const std::vector<Point>& polygon, Point axis, float& min, float& max);

Contact sat(const std::vector<Point>& a, const std::vector<Point>& b);

}
