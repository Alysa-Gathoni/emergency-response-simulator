#pragma once
#include <vector>
#include "../core/Point.h"

struct ClipRect {
    float left;
    float top;
    float right;
    float bottom;
};

namespace Clipping {
    // Sutherland-Hodgman polygon clipping against an axis-aligned rectangle.
    std::vector<Point> sutherlandHodgman(
        const std::vector<Point>& polygon,
        const ClipRect& rect
    );
}
