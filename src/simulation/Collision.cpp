#include "Collision.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace Collision {

AABB boundsOf(const std::vector<Point>& points) {
    if (points.empty()) return {0, 0, 0, 0};
    AABB box{std::numeric_limits<float>::max(), std::numeric_limits<float>::max(),
             std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest()};
    for (const Point& p : points) {
        box.minX = std::min(box.minX, p.x);
        box.minY = std::min(box.minY, p.y);
        box.maxX = std::max(box.maxX, p.x);
        box.maxY = std::max(box.maxY, p.y);
    }
    return box;
}

bool overlaps(const AABB& a, const AABB& b) {
    return a.minX < b.maxX && b.minX < a.maxX &&
           a.minY < b.maxY && b.minY < a.maxY;
}

Point minimumTranslation(const AABB& a, const AABB& b) {
    if (!overlaps(a, b)) return {0, 0};

    // Penetration if we push a to the left/right/up/down of b.
    const float pushLeft = b.minX - a.maxX;   // negative
    const float pushRight = b.maxX - a.minX;  // positive
    const float pushUp = b.minY - a.maxY;     // negative
    const float pushDown = b.maxY - a.minY;   // positive

    const float dx = -pushLeft < pushRight ? pushLeft : pushRight;
    const float dy = -pushUp < pushDown ? pushUp : pushDown;

    if (std::abs(dx) < std::abs(dy)) return {dx, 0};
    return {0, dy};
}

Point keepInside(const AABB& box, const AABB& world) {
    Point fix{0, 0};
    if (box.minX < world.minX) fix.x = world.minX - box.minX;
    else if (box.maxX > world.maxX) fix.x = world.maxX - box.maxX;
    if (box.minY < world.minY) fix.y = world.minY - box.minY;
    else if (box.maxY > world.maxY) fix.y = world.maxY - box.maxY;
    return fix;
}

}
