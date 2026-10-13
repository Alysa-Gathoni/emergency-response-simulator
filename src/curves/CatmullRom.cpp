#include "CatmullRom.h"
#include <algorithm>

namespace CatmullRom {

std::vector<BezierSegment> toBezier(const std::vector<Point>& knots,
                                    bool closed) {
    std::vector<BezierSegment> segments;
    const long n = static_cast<long>(knots.size());
    if (n < 2) return segments;

    // Closed paths wrap around; open paths repeat their end knots.
    auto knot = [&](long i) {
        if (closed) return knots[static_cast<std::size_t>(((i % n) + n) % n)];
        return knots[static_cast<std::size_t>(std::clamp(i, 0L, n - 1))];
    };

    const long spans = closed ? n : n - 1;
    for (long i = 0; i < spans; ++i) {
        const Point p0 = knot(i - 1);
        const Point p1 = knot(i);
        const Point p2 = knot(i + 1);
        const Point p3 = knot(i + 2);

        segments.push_back({
            p1,
            {p1.x + (p2.x - p0.x) / 6.0f, p1.y + (p2.y - p0.y) / 6.0f},
            {p2.x - (p3.x - p1.x) / 6.0f, p2.y - (p3.y - p1.y) / 6.0f},
            p2
        });
    }

    return segments;
}

}
