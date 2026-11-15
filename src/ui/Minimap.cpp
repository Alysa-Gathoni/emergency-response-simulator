#include "Minimap.h"
#include "../city/City.h"
#include "../polygons/Polygon.h"
#include "../rasterization/PolygonFill.h"
#include "../shapes/CircleEllipse.h"
#include <algorithm>
#include <cmath>

namespace {

const Rgba PanelColour(12, 16, 22, 220);
const Rgba FrameColour(90, 100, 112);
const Rgba BlockColour(58, 64, 72);
const Rgba RoadColour(96, 100, 108);
const Rgba ParkColour(46, 92, 58);
const Rgba Hospital(200, 205, 215);
const Rgba HospitalCross(220, 50, 50);
const Rgba TrafficDot(210, 210, 210);
const Rgba ViewFrame(240, 200, 80);

std::vector<Point> rect(float l, float t, float r, float b) {
    return {{l, t}, {r, t}, {r, b}, {l, b}};
}

}

Mat3 Minimap::windowToViewport(const ClipRect& w, const ClipRect& v) {
    const float ww = w.right - w.left, wh = w.bottom - w.top;
    const float vw = v.right - v.left, vh = v.bottom - v.top;
    const float s = std::min(vw / ww, vh / wh);
    return Mat3::translation((v.left + v.right) * 0.5f, (v.top + v.bottom) * 0.5f)
         * Mat3::scaling(s, s)
         * Mat3::translation(-(w.left + w.right) * 0.5f, -(w.top + w.bottom) * 0.5f);
}

Minimap::Minimap(const City& city, ClipRect viewport)
    : city_(city), viewport_(viewport) {
    const RoadNetwork& n = city.network();
    // Inset by a few pixels so the frame stays visible round the map.
    const ClipRect inner{viewport.left + 4, viewport.top + 4, viewport.right - 4, viewport.bottom - 4};
    transform_ = windowToViewport({0, 0, n.width(), n.height()}, inner);
}

void Minimap::draw(IRenderer& target, LineRenderer& lines, const Frame& f) const {
    if (!visible_) return;
    const Mat3& M = transform_;
    const RoadNetwork& network = city_.network();
    const ClipRect& v = viewport_;

    // Everything is cut to the minimap viewport (Cohen-Sutherland for
    // lines, Sutherland-Hodgman for polygons).
    const ClipRect* previous = lines.clip();
    const ClipRect saved = previous ? *previous : ClipRect{};
    lines.setClip(v);

    PolygonFill::draw(target, rect(v.left, v.top, v.right, v.bottom), lines, PanelColour, false);

    auto fillWorld = [&](const std::vector<Point>& world, Rgba c) {
        PolygonFill::draw(target, M.apply(world), lines, c, false);
    };

    for (const Rect& b : network.blocks()) fillWorld(rect(b.left, b.top, b.right, b.bottom), BlockColour);
    for (const Road& road : network.roads()) {
        const float lo = road.centre - RoadNetwork::HalfWidth, hi = road.centre + RoadNetwork::HalfWidth;
        fillWorld(road.vertical ? rect(lo, 0, hi, network.height()) : rect(0, lo, network.width(), hi), RoadColour);
    }
    for (const Rect& p : city_.parks()) fillWorld(rect(p.left, p.top, p.right, p.bottom), ParkColour);
    fillWorld(city_.hospital().vertices(), Hospital);
    const Point h = M.apply(city_.hospital().centroid());
    lines.draw(target, {h.x - 4, h.y}, {h.x + 4, h.y}, HospitalCross, 2.0f);
    lines.draw(target, {h.x, h.y - 4}, {h.x, h.y + 4}, HospitalCross, 2.0f);

    // Traffic as single dots.
    std::vector<Point> dots;
    for (const Point& p : f.traffic) {
        const Point q = M.apply(p);
        if (q.x >= v.left && q.x < v.right && q.y >= v.top && q.y < v.bottom)
            dots.push_back({q.x - 1.0f, q.y - 1.0f});
    }
    ShapeAlgorithms::drawPoints(target, dots, TrafficDot, 2.0f);

    // Route and target.
    for (std::size_t i = 1; i < f.route.size(); ++i)
        lines.draw(target, M.apply(f.route[i - 1]), M.apply(f.route[i]), f.routeColour, 2.0f);
    if (f.showTarget) {
        const Point t = M.apply(f.target);
        const int r = 4 + static_cast<int>(std::lround(2.0f + 2.0f * std::sin(f.time * 6.0f)));
        ShapeAlgorithms::drawPoints(target, ShapeAlgorithms::midpointCircle(t, r), f.routeColour, 2.0f);
        ShapeAlgorithms::drawPoints(target, ShapeAlgorithms::filledCircle(t, 2), f.routeColour, 1.0f);
    }

    // Camera view: the part of the map that is on screen.
    const Point c0 = M.apply(Point{f.cameraView.left, f.cameraView.top});
    const Point c1 = M.apply(Point{f.cameraView.right, f.cameraView.bottom});
    Polygon(rect(c0.x, c0.y, c1.x, c1.y)).drawOutline(target, lines, ViewFrame, 1.0f);

    // Ambulance: an arrow of fixed screen size, rotated to its heading.
    const Point a = M.apply(f.ambulance);
    const Mat3 place = Mat3::translation(a.x, a.y) * Mat3::rotationDegrees(f.headingDegrees);
    const std::vector<Point> arrow = place.apply(std::vector<Point>{{7, 0}, {-5, -5}, {-2, 0}, {-5, 5}});
    PolygonFill::draw(target, arrow, lines, Rgba(255, 255, 255), false);

    Polygon(rect(v.left, v.top, v.right, v.bottom)).drawOutline(target, lines, FrameColour, 1.0f);

    if (previous) lines.setClip(saved);
    else lines.clearClip();
}
