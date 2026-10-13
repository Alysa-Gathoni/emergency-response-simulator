#pragma once
#include <vector>
#include "../core/Point.h"
#include "Spline.h"

namespace CatmullRom {
    // A uniform Catmull-Rom span between knots p1 and p2 is
    //   C(t) = 0.5 * [ 2p1 + (p2 - p0)t + (2p0 - 5p1 + 4p2 - p3)t^2
    //                 + (3p1 - p0 - 3p2 + p3)t^3 ]
    // which is exactly the cubic Bezier with control points
    //   B0 = p1, B1 = p1 + (p2 - p0)/6, B2 = p2 - (p3 - p1)/6, B3 = p2.
    // The resulting path passes through every knot with C1 continuity.
    std::vector<BezierSegment> toBezier(const std::vector<Point>& knots,
                                        bool closed);
}
