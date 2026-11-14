#include "Triangulation.h"
#include <cmath>

namespace Triangulation {

namespace {
constexpr float Epsilon = 1e-4f;
}

float signedArea(const std::vector<Point>& polygon) {
    float twice = 0.0f;
    const std::size_t n = polygon.size();
    for (std::size_t i = 0; i < n; ++i) {
        const Point& a = polygon[i];
        const Point& b = polygon[(i + 1) % n];
        twice += a.x * b.y - b.x * a.y;
    }
    return twice * 0.5f;
}

float cross(Point a, Point b, Point c) {
    return (b.x - a.x) * (c.y - b.y) - (b.y - a.y) * (c.x - b.x);
}

bool insideTriangle(Point p, Point a, Point b, Point c) {
    // p is inside when it is on the same side of all three edges.
    const float d1 = cross(a, b, p);
    const float d2 = cross(b, c, p);
    const float d3 = cross(c, a, p);
    const bool hasNegative = d1 < -Epsilon || d2 < -Epsilon || d3 < -Epsilon;
    const bool hasPositive = d1 > Epsilon || d2 > Epsilon || d3 > Epsilon;
    return !(hasNegative && hasPositive);
}

std::vector<Triangle> earClip(const std::vector<Point>& polygon) {
    std::vector<Triangle> result;

    // Working list of vertex indices, without repeated points.
    std::vector<int> v;
    for (int i = 0; i < static_cast<int>(polygon.size()); ++i) {
        if (!v.empty()) {
            const Point& p = polygon[static_cast<std::size_t>(v.back())];
            if (std::abs(p.x - polygon[i].x) < Epsilon && std::abs(p.y - polygon[i].y) < Epsilon)
                continue;
        }
        v.push_back(i);
    }
    while (v.size() > 1) {
        const Point& first = polygon[static_cast<std::size_t>(v.front())];
        const Point& last = polygon[static_cast<std::size_t>(v.back())];
        if (std::abs(first.x - last.x) < Epsilon && std::abs(first.y - last.y) < Epsilon)
            v.pop_back();
        else
            break;
    }
    // Vertices on the straight line between their neighbours add nothing
    // but thin slivers: remove them first.
    for (bool changed = true; changed && v.size() >= 3;) {
        changed = false;
        for (std::size_t k = 0; k < v.size() && v.size() >= 3; ++k) {
            const Point& a = polygon[static_cast<std::size_t>(v[(k + v.size() - 1) % v.size()])];
            const Point& b = polygon[static_cast<std::size_t>(v[k])];
            const Point& c = polygon[static_cast<std::size_t>(v[(k + 1) % v.size()])];
            if (std::abs(cross(a, b, c)) <= Epsilon) {
                v.erase(v.begin() + static_cast<std::ptrdiff_t>(k));
                changed = true;
                break;
            }
        }
    }
    if (v.size() < 3) return result;

    // Convex corners turn the same way as the whole polygon.
    const float winding = signedArea(polygon) >= 0.0f ? 1.0f : -1.0f;
    auto at = [&](std::size_t k) { return polygon[static_cast<std::size_t>(v[k % v.size()])]; };

    std::size_t i = 0;
    std::size_t sinceLastEar = 0;
    while (v.size() > 3) {
        const std::size_t n = v.size();
        const std::size_t ia = (i + n - 1) % n, ib = i % n, ic = (i + 1) % n;
        const Point a = at(ia), b = at(ib), c = at(ic);
        const float turn = cross(a, b, c) * winding;

        bool ear = false;
        if (std::abs(turn) <= Epsilon) {
            // b is on the line a-c: dropping it changes nothing.
            v.erase(v.begin() + static_cast<std::ptrdiff_t>(ib));
            sinceLastEar = 0;
            continue;
        }
        if (turn > 0.0f) {
            ear = true;
            for (std::size_t k = 0; k < n && ear; ++k) {
                if (k == ia || k == ib || k == ic) continue;
                const Point p = at(k);
                // A vertex shared with the ear (same position) does not block it.
                const bool sameAsCorner =
                    (std::abs(p.x - a.x) < Epsilon && std::abs(p.y - a.y) < Epsilon) ||
                    (std::abs(p.x - c.x) < Epsilon && std::abs(p.y - c.y) < Epsilon);
                if (!sameAsCorner && insideTriangle(p, a, b, c)) ear = false;
            }
        }

        if (ear) {
            result.push_back({v[ia], v[ib], v[ic]});
            v.erase(v.begin() + static_cast<std::ptrdiff_t>(ib));
            sinceLastEar = 0;
            i = ib == 0 ? 0 : ib - 1;
            continue;
        }

        // A full lap without an ear only happens for self-intersecting
        // input (e.g. a badly clipped sliver): cut the corner anyway so the
        // loop always ends.
        if (++sinceLastEar > n) {
            result.push_back({v[ia], v[ib], v[ic]});
            v.erase(v.begin() + static_cast<std::ptrdiff_t>(ib));
            sinceLastEar = 0;
            continue;
        }
        i = (i + 1) % n;
    }

    if (std::abs(cross(at(0), at(1), at(2))) > Epsilon)
        result.push_back({v[0], v[1], v[2]});
    return result;
}

std::vector<Point> triangles(const std::vector<Point>& polygon) {
    std::vector<Point> out;
    for (const Triangle& t : earClip(polygon))
        for (int k : t) out.push_back(polygon[static_cast<std::size_t>(k)]);
    return out;
}

}
