#include "Clipping.h"

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

namespace {
constexpr int Inside = 0;
constexpr int Left   = 1;
constexpr int Right  = 2;
constexpr int Top    = 4;
constexpr int Bottom = 8;

int outCode(const Point& p, const ClipRect& r) {
    int code = Inside;
    if (p.x < r.left)        code |= Left;
    else if (p.x > r.right)  code |= Right;
    if (p.y < r.top)         code |= Top;
    else if (p.y > r.bottom) code |= Bottom;
    return code;
}
}

bool cohenSutherland(Point& a, Point& b, const ClipRect& rect) {
    int codeA = outCode(a, rect);
    int codeB = outCode(b, rect);

    while (true) {
        if ((codeA | codeB) == 0) return true;   // trivially accepted
        if ((codeA & codeB) != 0) return false;  // trivially rejected

        // Pick an endpoint that is outside and move it onto the
        // boundary edge indicated by its region code.
        const int outside = codeA != 0 ? codeA : codeB;
        const float dx = b.x - a.x;
        const float dy = b.y - a.y;
        Point p;

        if (outside & Top) {
            p = {a.x + dx * (rect.top - a.y) / dy, rect.top};
        } else if (outside & Bottom) {
            p = {a.x + dx * (rect.bottom - a.y) / dy, rect.bottom};
        } else if (outside & Right) {
            p = {rect.right, a.y + dy * (rect.right - a.x) / dx};
        } else {
            p = {rect.left, a.y + dy * (rect.left - a.x) / dx};
        }

        if (outside == codeA) {
            a = p;
            codeA = outCode(a, rect);
        } else {
            b = p;
            codeB = outCode(b, rect);
        }
    }
}

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
