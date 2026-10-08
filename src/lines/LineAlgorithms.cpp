#include "LineAlgorithms.h"
#include <algorithm>
#include <cmath>

namespace LineAlgorithms {

std::vector<Point> dda(Point start, Point end) {
    std::vector<Point> points;

    const float dx = end.x - start.x;
    const float dy = end.y - start.y;
    const int steps = static_cast<int>(std::max(std::abs(dx), std::abs(dy)));

    if (steps == 0) {
        points.push_back(start);
        return points;
    }

    const float xIncrement = dx / steps;
    const float yIncrement = dy / steps;

    float x = start.x;
    float y = start.y;

    for (int i = 0; i <= steps; ++i) {
        points.emplace_back(std::round(x), std::round(y));
        x += xIncrement;
        y += yIncrement;
    }

    return points;
}

std::vector<Point> bresenham(Point start, Point end) {
    std::vector<Point> points;

    int x1 = static_cast<int>(std::round(start.x));
    int y1 = static_cast<int>(std::round(start.y));
    const int x2 = static_cast<int>(std::round(end.x));
    const int y2 = static_cast<int>(std::round(end.y));

    const int dx = std::abs(x2 - x1);
    const int sx = x1 < x2 ? 1 : -1;
    const int dy = -std::abs(y2 - y1);
    const int sy = y1 < y2 ? 1 : -1;
    int error = dx + dy;

    while (true) {
        points.emplace_back(static_cast<float>(x1), static_cast<float>(y1));
        if (x1 == x2 && y1 == y2) break;

        const int doubledError = 2 * error;
        if (doubledError >= dy) {
            error += dy;
            x1 += sx;
        }
        if (doubledError <= dx) {
            error += dx;
            y1 += sy;
        }
    }

    return points;
}

}
