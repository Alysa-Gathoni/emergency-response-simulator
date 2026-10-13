#include "CircleEllipse.h"
#include "../core/Raster.h"
#include <algorithm>
#include <cmath>
#include <map>

namespace {

void addCircleSymmetry(std::vector<Point>& points,
                       Point c, int x, int y) {
    points.push_back({c.x + x, c.y + y});
    points.push_back({c.x - x, c.y + y});
    points.push_back({c.x + x, c.y - y});
    points.push_back({c.x - x, c.y - y});
    points.push_back({c.x + y, c.y + x});
    points.push_back({c.x - y, c.y + x});
    points.push_back({c.x + y, c.y - x});
    points.push_back({c.x - y, c.y - x});
}

void addEllipseSymmetry(std::vector<Point>& points,
                        Point c, int x, int y) {
    points.push_back({c.x + x, c.y + y});
    points.push_back({c.x - x, c.y + y});
    points.push_back({c.x + x, c.y - y});
    points.push_back({c.x - x, c.y - y});
}

// Circles and ellipses are convex, so every row of the outline has exactly
// one interior span between its minimum and maximum x.
std::vector<Point> fillOutlineRows(const std::vector<Point>& outline) {
    std::map<long, std::pair<float, float>> rows;

    for (const Point& p : outline) {
        const long y = std::lround(p.y);
        auto it = rows.find(y);
        if (it == rows.end()) {
            rows[y] = {p.x, p.x};
        } else {
            it->second.first = std::min(it->second.first, p.x);
            it->second.second = std::max(it->second.second, p.x);
        }
    }

    std::vector<Point> pixels;
    for (const auto& [y, span] : rows) {
        for (float x = span.first; x <= span.second; x += 1.0f)
            pixels.push_back({x, static_cast<float>(y)});
    }
    return pixels;
}

}

namespace ShapeAlgorithms {

std::vector<Point> midpointCircle(Point center, int radius) {
    std::vector<Point> points;
    if (radius < 0) return points;

    int x = 0;
    int y = radius;
    int decision = 1 - radius;

    addCircleSymmetry(points, center, x, y);

    while (x < y) {
        ++x;

        if (decision < 0) {
            decision += 2 * x + 1;
        } else {
            --y;
            decision += 2 * (x - y) + 1;
        }

        addCircleSymmetry(points, center, x, y);
    }

    return points;
}

std::vector<Point> midpointEllipse(Point center, int radiusX, int radiusY) {
    std::vector<Point> points;
    if (radiusX <= 0 || radiusY <= 0) return points;

    long long rx2 = 1LL * radiusX * radiusX;
    long long ry2 = 1LL * radiusY * radiusY;

    int x = 0;
    int y = radiusY;

    long long dx = 0;
    long long dy = 2 * rx2 * y;

    double p1 = ry2 - rx2 * radiusY + 0.25 * rx2;

    while (dx < dy) {
        addEllipseSymmetry(points, center, x, y);

        ++x;
        dx += 2 * ry2;

        if (p1 < 0) {
            p1 += dx + ry2;
        } else {
            --y;
            dy -= 2 * rx2;
            p1 += dx - dy + ry2;
        }
    }

    double p2 =
        ry2 * (x + 0.5) * (x + 0.5) +
        rx2 * (y - 1.0) * (y - 1.0) -
        rx2 * ry2;

    while (y >= 0) {
        addEllipseSymmetry(points, center, x, y);

        --y;
        dy -= 2 * rx2;

        if (p2 > 0) {
            p2 += rx2 - dy;
        } else {
            ++x;
            dx += 2 * ry2;
            p2 += dx - dy + rx2;
        }
    }

    return points;
}

std::vector<Point> filledCircle(Point center, int radius) {
    return fillOutlineRows(midpointCircle(center, radius));
}

std::vector<Point> filledEllipse(Point center, int radiusX, int radiusY) {
    return fillOutlineRows(midpointEllipse(center, radiusX, radiusY));
}

void drawPoints(sf::RenderTarget& target,
                const std::vector<Point>& points,
                sf::Color color,
                float pixelSize) {
    Raster::plot(target, points, color, pixelSize);
}

}
