#pragma once
#include <vector>
#include "../core/Point.h"

namespace Bezier {
    Point cubic(float t, Point p0, Point p1, Point p2, Point p3);

    std::vector<Point> sampleCubic(
        Point p0, Point p1, Point p2, Point p3,
        int segments = 80
    );
}
