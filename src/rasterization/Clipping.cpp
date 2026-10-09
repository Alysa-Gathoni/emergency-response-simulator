#include "Clipping.h"
#include <functional>

namespace {

enum class Edge { Left, Right, Top, Bottom };

bool inside(const Point& p, Edge edge, const ClipRect& r) {
    switch (edge) {
        case Edge::Left:   return p.x >= r.left;
        case Edge::Right:  return p.x <= r.right;
        case Edge::Top:    return p.y >= r.top;
        case Edge::Bottom: return p.y <= r.bottom;
    }
    return false;
}

Point intersection(Point a, Point b, Edge edge, const ClipRect& r) {
    const float dx = b.x - a.x;
    const float dy = b.y - a.y;

    switch (edge) {
        case Edge::Left: {
            const float t = (r.left - a.x) / dx;
            return {r.left, a.y + t * dy};
        }
        case Edge::Right: {
            const float t = (r.right - a.x) / dx;
            return {r.right, a.y + t * dy};
        }
        case Edge::Top: {
            const float t = (r.top - a.y) / dy;
            return {a.x + t * dx, r.top};
        }
        case Edge::Bottom: {
            const float t = (r.bottom - a.y) / dy;
            return {a.x + t * dx, r.bottom};
        }
    }
    return a;
}

std::vector<Point> clipAgainst(
    const std::vector<Point>& input,
    Edge edge,
    const ClipRect& rect
) {
    std::vector<Point> output;
    if (input.empty()) return output;

    Point previous = input.back();
    bool previousInside = inside(previous, edge, rect);

    for (const Point& current : input) {
        const bool currentInside = inside(current, edge, rect);

        if (currentInside) {
            if (!previousInside)
                output.push_back(intersection(previous, current, edge, rect));
            output.push_back(current);
        } else if (previousInside) {
            output.push_back(intersection(previous, current, edge, rect));
        }

        previous = current;
        previousInside = currentInside;
    }

    return output;
}

}

namespace Clipping {

std::vector<Point> sutherlandHodgman(
    const std::vector<Point>& polygon,
    const ClipRect& rect
) {
    std::vector<Point> output = polygon;
    output = clipAgainst(output, Edge::Left, rect);
    output = clipAgainst(output, Edge::Right, rect);
    output = clipAgainst(output, Edge::Top, rect);
    output = clipAgainst(output, Edge::Bottom, rect);
    return output;
}

}
