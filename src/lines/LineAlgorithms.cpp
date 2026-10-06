#include "lines/LineAlgorithms.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>

std::vector<Point> LineAlgorithms::dda(int x0, int y0, int x1, int y1) {
    std::vector<Point> points;
    const int dx = x1 - x0;
    const int dy = y1 - y0;
    const int steps = std::max(std::abs(dx), std::abs(dy));
    if (steps == 0) return {{x0, y0}};
    const float xInc = static_cast<float>(dx) / steps;
    const float yInc = static_cast<float>(dy) / steps;
    float x = static_cast<float>(x0), y = static_cast<float>(y0);
    points.reserve(static_cast<std::size_t>(steps + 1));
    for (int i = 0; i <= steps; ++i) {
        points.push_back({static_cast<int>(std::lround(x)), static_cast<int>(std::lround(y))});
        x += xInc; y += yInc;
    }
    return points;
}

std::vector<Point> LineAlgorithms::bresenham(int x0, int y0, int x1, int y1) {
    std::vector<Point> points;
    const int dx = std::abs(x1 - x0);
    const int sx = x0 < x1 ? 1 : -1;
    const int dy = -std::abs(y1 - y0);
    const int sy = y0 < y1 ? 1 : -1;
    int error = dx + dy;
    while (true) {
        points.push_back({x0, y0});
        if (x0 == x1 && y0 == y1) break;
        const int e2 = 2 * error;
        if (e2 >= dy) { error += dy; x0 += sx; }
        if (e2 <= dx) { error += dx; y0 += sy; }
    }
    return points;
}
