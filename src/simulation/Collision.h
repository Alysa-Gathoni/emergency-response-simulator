#pragma once
#include <vector>
#include "../core/Point.h"

// Axis-aligned bounding boxes (pure C++, no rendering).
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

}
