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

void project(const std::vector<Point>& polygon, Point axis, float& min, float& max) {
    min = std::numeric_limits<float>::max();
    max = std::numeric_limits<float>::lowest();
    for (const Point& p : polygon) {
        const float d = p.x * axis.x + p.y * axis.y;
        min = std::min(min, d);
        max = std::max(max, d);
    }
}

Contact sat(const std::vector<Point>& a, const std::vector<Point>& b) {
    Contact best;
    if (a.size() < 3 || b.size() < 3) return best;
    best.depth = std::numeric_limits<float>::max();

    // Candidate axes: the normal of every edge of both polygons.
    for (const std::vector<Point>* shape : {&a, &b}) {
        const std::vector<Point>& poly = *shape;
        for (std::size_t i = 0; i < poly.size(); ++i) {
            const Point& p = poly[i];
            const Point& q = poly[(i + 1) % poly.size()];
            Point axis{-(q.y - p.y), q.x - p.x};
            const float len = std::hypot(axis.x, axis.y);
            if (len < 1e-6f) continue;
            axis = {axis.x / len, axis.y / len};

            float minA, maxA, minB, maxB;
            project(a, axis, minA, maxA);
            project(b, axis, minB, maxB);
            const float overlap = std::min(maxA, maxB) - std::max(minA, minB);
            if (overlap <= 0.0f) return Contact{};    // separating axis found

            if (overlap < best.depth) {
                best.depth = overlap;
                best.normal = axis;
            }
        }
    }

    // Point the normal from b towards a (centroid to centroid).
    Point ca{0, 0}, cb{0, 0};
    for (const Point& p : a) { ca.x += p.x; ca.y += p.y; }
    for (const Point& p : b) { cb.x += p.x; cb.y += p.y; }
    ca = {ca.x / a.size(), ca.y / a.size()};
    cb = {cb.x / b.size(), cb.y / b.size()};
    if ((ca.x - cb.x) * best.normal.x + (ca.y - cb.y) * best.normal.y < 0.0f)
        best.normal = {-best.normal.x, -best.normal.y};
    best.hit = true;
    return best;
}

}
