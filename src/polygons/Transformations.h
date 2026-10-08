#pragma once
#include <vector>
#include "../core/Point.h"

namespace Transformations {
    std::vector<Point> translate(const std::vector<Point>& vertices,
                                 float dx, float dy);

    std::vector<Point> rotate(const std::vector<Point>& vertices,
                              float degrees, Point pivot);

    std::vector<Point> scale(const std::vector<Point>& vertices,
                             float sx, float sy, Point pivot);
}
