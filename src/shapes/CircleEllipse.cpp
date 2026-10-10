#include "CircleEllipse.h"
#include <cmath>

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

void drawPoints(sf::RenderTarget& target,
                const std::vector<Point>& points,
                sf::Color color,
                float pixelSize) {
    sf::RectangleShape pixel({pixelSize, pixelSize});
    pixel.setFillColor(color);

    for (const Point& p : points) {
        pixel.setPosition(p.x, p.y);
        target.draw(pixel);
    }
}

}
