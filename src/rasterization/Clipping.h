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
    // Cohen-Sutherland line clipping against an axis-aligned rectangle.
    // Returns false when the segment lies completely outside; otherwise
    // a and b are moved onto the visible part of the segment.
    bool cohenSutherland(Point& a, Point& b, const ClipRect& rect);

    // Sutherland-Hodgman polygon clipping against an axis-aligned rectangle.
    std::vector<Point> sutherlandHodgman(
        const std::vector<Point>& polygon,
        const ClipRect& rect
    );
}
