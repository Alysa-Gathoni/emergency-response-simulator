#include "Transformations.h"
#include <cmath>

namespace {
constexpr float PI = 3.14159265358979323846f;
}

namespace Transformations {

std::vector<Point> translate(const std::vector<Point>& vertices,
                             float dx, float dy) {
    std::vector<Point> result;
    result.reserve(vertices.size());

    for (const Point& p : vertices)
        result.emplace_back(p.x + dx, p.y + dy);

    return result;
}

std::vector<Point> rotate(const std::vector<Point>& vertices,
                          float degrees, Point pivot) {
    std::vector<Point> result;
    result.reserve(vertices.size());

    const float radians = degrees * PI / 180.0f;
    const float c = std::cos(radians);
    const float s = std::sin(radians);

    for (const Point& p : vertices) {
        const float x = p.x - pivot.x;
        const float y = p.y - pivot.y;

        result.emplace_back(
            pivot.x + x * c - y * s,
            pivot.y + x * s + y * c
        );
    }

    return result;
}

std::vector<Point> scale(const std::vector<Point>& vertices,
                         float sx, float sy, Point pivot) {
    std::vector<Point> result;
    result.reserve(vertices.size());

    for (const Point& p : vertices) {
        result.emplace_back(
            pivot.x + (p.x - pivot.x) * sx,
            pivot.y + (p.y - pivot.y) * sy
        );
    }

    return result;
}

}
