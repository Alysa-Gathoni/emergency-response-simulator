#include "Bezier.h"
#include <algorithm>

namespace Bezier {

Point cubic(float t, Point p0, Point p1, Point p2, Point p3) {
    t = std::clamp(t, 0.0f, 1.0f);
    const float u = 1.0f - t;

    const float b0 = u * u * u;
    const float b1 = 3.0f * u * u * t;
    const float b2 = 3.0f * u * t * t;
    const float b3 = t * t * t;

    return {
        b0 * p0.x + b1 * p1.x + b2 * p2.x + b3 * p3.x,
        b0 * p0.y + b1 * p1.y + b2 * p2.y + b3 * p3.y
    };
}

std::vector<Point> sampleCubic(
    Point p0, Point p1, Point p2, Point p3,
    int segments
) {
    segments = std::max(1, segments);
    std::vector<Point> points;
    points.reserve(static_cast<std::size_t>(segments) + 1);

    for (int i = 0; i <= segments; ++i) {
        const float t = static_cast<float>(i) / segments;
        points.push_back(cubic(t, p0, p1, p2, p3));
    }
    return points;
}

}
