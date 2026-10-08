#pragma once
#include <vector>
#include "../core/Point.h"

namespace LineAlgorithms {
    std::vector<Point> dda(Point start, Point end);
    std::vector<Point> bresenham(Point start, Point end);
}
